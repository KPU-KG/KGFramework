#include "pch.h"
#include "fbxImpoter.h"
#include "GameObject.h"
#include "MathHelper.h"
#include "ResourceContainer.h"

#include "Scene.h"
#include "FakeAnimationComponent.h"
#include "FakeGeometryComponent.h"

#include <iostream>

using namespace DirectX;

void KG::Component::FakeBoneTransformComponent::OnCreate(KG::Core::GameObject* gameObject)
{
    IRenderComponent::OnCreate(gameObject);
    auto* tran = this->GetGameObject()->GetTransform();
    while ( rootNode == nullptr && tran != nullptr )
    {
        auto* obj = tran->GetGameObject();
        if ( this->rootNodeTag == obj->tag )
        {
            this->SetRootNode(obj);
        }
        else
        {
            tran = tran->GetParent();
        }
    }
    assert(rootNode != nullptr && "Root Node is Null");
    this->InitializeBone(rootNode);
}

KG::Component::FakeBoneTransformComponent::FakeBoneTransformComponent()
    : rootNodeIdProp("RootNodeID", rootNodeTag)
{
}

KG::Core::GameObject* KG::Component::FakeBoneTransformComponent::BoneIndexToGameObject(UINT index, UINT submeshIndex) const
{
    return this->frameCache[submeshIndex][index];
}

void KG::Component::FakeBoneTransformComponent::SetRootNode(KG::Core::GameObject* object)
{
    rootNode = object;
    this->rootNodeTag = rootNode->tag;
}

void KG::Component::FakeBoneTransformComponent::InitializeBone(KG::Core::GameObject* rootNode)
{
    this->geometry = gameObject->GetComponent<KG::Component::FakeGeometryComponent>();

    this->frameCache.resize(this->geometry->geometrys.size());
    for ( size_t i = 0; i < this->geometry->geometrys.size(); i++ )
    {
        auto& boneIds = this->geometry->geometrys[i]->boneIds;
        auto& cache = this->frameCache[i];
        cache.resize(boneIds.size());
        rootNode->MatchBoneToObject(boneIds, cache);
#ifdef _DEBUG
        for ( auto& i : cache )
        {
            if ( i == nullptr )
            {
                DebugErrorMessage("Bone Not Linked Object!");
            }
        }
#endif
    }
}

void KG::Component::FakeBoneTransformComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
    this->rootNodeIdProp.OnDataLoad(componentElement);
}

void KG::Component::FakeBoneTransformComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
    auto* componentElement = parentElement->InsertNewChildElement("Component");
    ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::FakeBoneTransformComponent);
    this->rootNodeIdProp.OnDataSave(componentElement);
}

bool KG::Component::FakeBoneTransformComponent::OnDrawGUI()
{
    if ( ImGui::ComponentHeader<FakeBoneTransformComponent>() )
    {
        this->rootNodeIdProp.OnDrawGUI();
    }
    return false;
}


static float GetTimeData(const std::vector<KG::Utill::KeyData>& data, float currentTime, float duration, float defaultValue = 0.0f)
{
    float value = defaultValue;
    if ( !data.empty() )
    {
        value = data[0].value;
        KG::Utill::KeyData tempData;
        tempData.keyTime = currentTime;
        tempData.value = 0.0f;
        auto p = std::equal_range(data.begin(), data.end(), tempData);
        auto prev = p.first != data.begin() ? std::prev(p.first) : p.first;
        auto last = p.second;

        float keyTime0 = 0.0f;
        float keyTime1 = 0.0f;
        float keyValue0 = 0.0f;
        float keyValue1 = 0.0f;

        if ( currentTime == duration )
            return data.back().value;

        if ( prev == data.end() )
        {
            keyTime0 = data.back().keyTime;
            keyValue0 = data.back().value;
        }
        //else if ( prev == data.begin() )
        //{
        //	keyTime0 = data.back().keyTime;
        //	keyValue0 = data.back().value;
        //}
        else
        {
            keyTime0 = prev->keyTime;
            keyValue0 = prev->value;
        }

        if ( last == data.end() )
        {
            keyTime1 = duration;
            keyValue1 = value;
        }
        else
        {
            while ( last != data.end() && keyTime1 <= keyTime0 )
            {
                keyTime1 = last->keyTime;
                keyValue1 = last->value;
                last++;
            }
        }
        if ( keyTime0 == keyTime1 )
            return keyValue1;
        value = KG::Math::Lerp(keyValue0, keyValue1, abs(currentTime - keyTime0) / abs(keyTime1 - keyTime0));
        //value = keyValue0;
    }
    return value;
}

static XMFLOAT3 GetAnimationTranslation(const KG::Utill::NodeAnimation& anim, float currentTime, float duration)
{
    XMFLOAT3 t = {};
    if ( anim.translation.x.size() <= 0 && anim.translation.y.size() <= 0 && anim.translation.z.size() <= 0 )
    {
        t = { 0,0,0 };
    }
    else
    {
        t.x = GetTimeData(anim.translation.x, currentTime, duration);
        t.y = GetTimeData(anim.translation.y, currentTime, duration);
        t.z = GetTimeData(anim.translation.z, currentTime, duration);
    }
    return t;
}

static XMFLOAT4 GetAnimationRotation(const KG::Utill::NodeAnimation& anim, float currentTime, float duration)
{
    XMFLOAT3 r = {};

    r.x = GetTimeData(anim.rotation.x, currentTime, duration);
    r.y = GetTimeData(anim.rotation.y, currentTime, duration);
    r.z = GetTimeData(anim.rotation.z, currentTime, duration);

    r.x = XMConvertToRadians(r.x);
    r.y = XMConvertToRadians(r.y);
    r.z = XMConvertToRadians(r.z);
    XMFLOAT4 rQuat = KG::Math::Quaternion::Multiply(KG::Math::Quaternion::FromXYZEuler(r), anim.preRotation);
    return rQuat;
}

static XMFLOAT3 GetAnimationScale(const KG::Utill::NodeAnimation& anim, float currentTime, float duration)
{
    XMFLOAT3 s = {};
    s.x = GetTimeData(anim.scale.x, currentTime, duration, 1.0f);
    s.y = GetTimeData(anim.scale.y, currentTime, duration, 1.0f);
    s.z = GetTimeData(anim.scale.z, currentTime, duration, 1.0f);
    return s;
}

int KG::Component::FakeAnimationControllerComponent::GetTotalWeight(int index)
{
    if ( index == -1 )
    {
        int total = 0;
        for ( auto& idx : curAnimation.index )
        {
            for ( auto& w : idx.second )
            {
                total += w.second;
            }
        }
        return total;
    }
    else if ( index < nextAnimations.size() )
    {
        int total = 0;
        for ( auto& idx : nextAnimations[index].index )
        {
            for ( auto& w : idx.second )
                total += w.second;
        }
        return total;
    }
    return 1;
}


void KG::Component::FakeAnimationControllerComponent::OnCreate(KG::Core::GameObject* gameObject)
{
    for ( auto& animation : animations )
    {
        for ( auto& anim : animation.second )
        {
            anim.second.Initialize(this->gameObject, anim.first);
        }
    }
}

bool KG::Component::FakeAnimationControllerComponent::IsValidAnimationId(const KG::Utill::HashString& animationId, UINT animIndex)
{
    if ( animations.count(animationId.value) == 0 || animations[animationId.value].count(animIndex) == 0 )
        return false;
    else
        return true;
}

void KG::Component::FakeAnimationControllerComponent::OnDestroy()
{

}

void KG::Component::FakeAnimationControllerComponent::PlayingUpdate(float elapsedTime)
{
    if ( curAnimation.index.size() <= 0 )
        return;
    auto p = curAnimation.index.begin();
    Animation* anim = &animations[p->first][p->second.begin()->first];
    // curAnimation.index.


    anim->timer += elapsedTime * curAnimation.speed;

    if ( anim->timer >= anim->duration )
    {
        anim->timer -= anim->duration;
        if ( curAnimation.repeat != ANIMLOOP_INF )
            curAnimation.repeat--;
        // 한 애니메이션 넘어가면 이벤트 초기화
        // for (int idx : curAnimation.index) {
        // 	if (events[animations[idx].animationId].size() > 0) {
        // 		for (auto& e : events[animations[idx].animationId]) {
        // 			e.activated = false;
        // 		}
        // 	}
        // }
    }

    // sound event
    // 현재는 검증은 불가능
    // for (int idx : curAnimation.index) {
    // 	if (events[animations[idx].animationId].size() > 0) {
    // 		for (auto& e : events[animations[idx].animationId]) {
    // 			// 요 상수값은 바꿔야 합니다...
    // 			if (e.time >= animations[idx].timer && e.time - animations[idx].timer < 0.07f) {
    // 				// KG::Utilll::SoundPlay 나중에 추가될 사운드 이벤트 입니다
    // 				e.activated = true;
    // 			}
    // 		}
    // 	}
    // }

    if ( curAnimation.repeat == ANIMLOOP_INF )
        ; // loop inf
    else if ( curAnimation.repeat <= 0 )
    {
        if ( state == ANIMSTATE_STOP )
            ;
        else
        {
            float animT = animations[p->first][p->second.begin()->first].timer / animations[p->first][p->second.begin()->first].duration;
            if ( nextAnimations.size() <= 0 )
            {
                animations[defaultAnimation.first][defaultAnimation.second].timer = animT * animations[defaultAnimation.first][defaultAnimation.second].duration;
                state = ANIMSTATE_CHANGING;
                ChangeAnimation(defaultAnimation.first, defaultAnimation.second, ANIMSTATE_PLAYING, 0.5f, -1);
                changeToDefault = true;
            }
            else
            {
                auto pn = nextAnimations[0].index.begin();
                animations[pn->first][pn->second.begin()->first].timer = animT * animations[pn->first][pn->second.begin()->first].duration;
                curAnimation = nextAnimations[0];
                curAnimation.time = 0;
                nextAnimations.erase(nextAnimations.begin());
                state = curAnimation.next;
            }
        }
        return;
    }


    float T = anim->timer / anim->duration;
    if ( state == ANIMSTATE_STOP && curAnimation.repeat <= 0 )
        T = 1;
    curAnimation.time += elapsedTime * curAnimation.speed;

    KG::Utill::AnimationSet* animSet = nullptr;

    std::vector<std::vector<DirectX::XMFLOAT3>> t;
    std::vector<std::vector<DirectX::XMFLOAT4>> r;
    std::vector<std::vector<DirectX::XMFLOAT3>> s;
    t.resize(curAnimation.index.size());
    r.resize(curAnimation.index.size());
    s.resize(curAnimation.index.size());

    int animCount = 0;
    int totalWeight = GetTotalWeight(ANIMINDEX_CURRENT);
    for ( auto& idx : curAnimation.index )
    {
        for ( auto& in : idx.second )
        {
            anim = &animations[idx.first][in.first];
            auto* inst = KG::Resource::ResourceContainer::GetInstance();

            anim->timer = T * anim->duration;

            animSet = inst->LoadAnimation(anim->animationId, in.first);
            t[animCount].resize(animSet->layers[0].nodeAnimations.size());
            r[animCount].resize(animSet->layers[0].nodeAnimations.size());
            s[animCount].resize(animSet->layers[0].nodeAnimations.size());
            for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
            {
                if ( animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id) )
                {
                    continue;
                }

                t[animCount][i] = GetAnimationTranslation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
                r[animCount][i] = GetAnimationRotation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
                s[animCount][i] = GetAnimationScale(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
            }
            animCount++;
        }
    }

    for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
    {
        if ( animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id) )
        {
            continue;
        }

        XMFLOAT3 pos = t[0][i];
        XMFLOAT4 rot = r[0][i];
        XMFLOAT3 scale = s[0][i];

        int count = 1;
        for ( auto iter = curAnimation.index.begin(); iter != curAnimation.index.end(); ++iter )
        {
            for ( auto it = iter->second.begin(); it != iter->second.end(); ++count )
            {
                float prevWeight = it++->second;
                if ( it == iter->second.end() )
                    break;
                float curWeight = it->second;
                float weight = curWeight / (curWeight + prevWeight);

                XMStoreFloat3(&pos, XMVectorLerp(XMLoadFloat3(&pos), XMLoadFloat3(&t[count][i]), weight));
                XMStoreFloat4(&rot, XMQuaternionSlerp(XMLoadFloat4(&rot), XMLoadFloat4(&r[count][i]), weight));
                XMStoreFloat3(&scale, XMVectorLerp(XMLoadFloat3(&scale), XMLoadFloat3(&s[count][i]), weight));
            }

            if ( anim->frameCache[0][i] != nullptr )
            {
                if ( curAnimation.applyTransform && !this->isIgnoreTranslate )
                    anim->frameCache[0][i]->GetTransform()->SetPosition(pos);
                if ( curAnimation.applyRotation )
                    anim->frameCache[0][i]->GetTransform()->SetRotation(rot);
                if ( curAnimation.applyScale && !this->isIgnoreScale )
                    anim->frameCache[0][i]->GetTransform()->SetScale(scale);
            }
        }
    }
    curFrame = anim;
}

void KG::Component::FakeAnimationControllerComponent::ChangingUpdate(float elapsedTime)
{
    if ( curAnimation.index.size() <= 0 )
        return;
    auto pc = curAnimation.index.begin();
    Animation* anim = &animations[pc->first][pc->second.begin()->first];
    // curAnimation.index.


    anim->timer += elapsedTime * curAnimation.speed;

    if ( anim->timer >= anim->duration )
    {
        anim->timer -= anim->duration;
        if ( curAnimation.repeat != ANIMLOOP_INF )
            curAnimation.repeat--;
        // 한 애니메이션 넘어가면 이벤트 초기화
        // for (int idx : curAnimation.index) {
        // 	if (events[animations[idx].animationId].size() > 0) {
        // 		for (auto& e : events[animations[idx].animationId]) {
        // 			e.activated = false;
        // 		}
        // 	}
        // }
    }

    // if (curAnimation.repeat == ANIMLOOP_INF)
    // 	; // loop inf
    if ( curAnimation.time >= curAnimation.duration )
    {
        if ( animations.size() < 10 )
            DebugNormalMessage("Animation Change Next"); // 알림점
    // else if (curAnimation.repeat <= 0) {
        float animT = animations[pc->first][pc->second.begin()->first].timer / animations[pc->first][pc->second.begin()->first].duration;
        if ( nextAnimations.size() <= 0 )
        {
            animations[defaultAnimation.first][defaultAnimation.second].timer = animT * animations[defaultAnimation.first][defaultAnimation.second].duration;
            state = ANIMSTATE_CHANGING;
            ChangeAnimation(defaultAnimation.first, defaultAnimation.second, ANIMSTATE_PLAYING, 0.5f, -1);
            changeToDefault = true;
            changeIntercepted = false;
        }
        else
        {
            auto pn = nextAnimations[0].index.begin();
            animations[pn->first][pn->second.begin()->first].timer = animT * animations[pn->first][pn->second.begin()->first].duration;
            curAnimation = nextAnimations[0];
            curAnimation.time = 0;
            nextAnimations.erase(nextAnimations.begin());
            state = curAnimation.next;
            // if (state == ANIMSTATE_STOP)
            // 	curAnimation.repeat;
        }
        return;
    }

    float T = anim->timer / anim->duration;
    if ( changeToDefault )
        T = 1;

    curAnimation.time += elapsedTime * curAnimation.speed;

    KG::Utill::AnimationSet* animSet = nullptr;

    std::vector<KG::Utill::HashString> prevIndexName;
    std::vector<std::vector<DirectX::XMFLOAT3>> t;
    std::vector<std::vector<DirectX::XMFLOAT4>> r;
    std::vector<std::vector<DirectX::XMFLOAT3>> s;
    t.resize(curAnimation.index.size());
    r.resize(curAnimation.index.size());
    s.resize(curAnimation.index.size());

    int animCount = 0;
    int totalWeight = GetTotalWeight(ANIMINDEX_CURRENT);
    for ( auto& idx : curAnimation.index )
    {
        for ( auto& in : idx.second )
        {
            anim = &animations[idx.first][in.first];
            auto* inst = KG::Resource::ResourceContainer::GetInstance();

            anim->timer = T * anim->duration;

            animSet = inst->LoadAnimation(anim->animationId, in.first);
            prevIndexName.resize(animSet->layers[0].nodeAnimations.size());
            t[animCount].resize(animSet->layers[0].nodeAnimations.size());
            r[animCount].resize(animSet->layers[0].nodeAnimations.size());
            s[animCount].resize(animSet->layers[0].nodeAnimations.size());
            for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
            {
                // if (prevIndexName[i]. == 0)
                prevIndexName[i] = animSet->layers[0].nodeAnimations[i].nodeId;
                if ( animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id) )
                {
                    continue;
                }
                t[animCount][i] = GetAnimationTranslation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
                r[animCount][i] = GetAnimationRotation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
                s[animCount][i] = GetAnimationScale(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
            }
            animCount++;
        }
    }
    std::vector<XMFLOAT3> pos;
    std::vector<XMFLOAT4> rot;
    std::vector<XMFLOAT3> scale;
    pos.resize(animSet->layers[0].nodeAnimations.size());
    rot.resize(animSet->layers[0].nodeAnimations.size());
    scale.resize(animSet->layers[0].nodeAnimations.size());

    for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
    {
        if ( animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id) )
        {
            continue;
        }

        pos[i] = t[0][i];
        rot[i] = r[0][i];
        scale[i] = s[0][i];

        int count = 1;
        for ( auto iter = curAnimation.index.begin(); iter != curAnimation.index.end(); ++iter )
        {
            for ( auto it = iter->second.begin(); it != iter->second.end(); count++ )
            {
                float prevWeight = it++->second;
                if ( it == iter->second.end() )
                    break;
                float curWeight = it->second;
                float weight = curWeight / (curWeight + prevWeight);

                XMStoreFloat3(&pos[i], XMVectorLerp(XMLoadFloat3(&pos[i]), XMLoadFloat3(&t[count][i]), weight));
                XMStoreFloat4(&rot[i], XMQuaternionSlerp(XMLoadFloat4(&rot[i]), XMLoadFloat4(&r[count][i]), weight));
                XMStoreFloat3(&scale[i], XMVectorLerp(XMLoadFloat3(&scale[i]), XMLoadFloat3(&s[count][i]), weight));
            }
        }
    }

    /// next animation 계산 (blending)

    if ( nextAnimations.size() <= 0 )
    {
        for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
        {
            if ( curAnimation.applyTransform )
                ;
            if ( curAnimation.applyRotation )
                anim->frameCache[0][i]->GetTransform()->SetRotation(rot[i]);
            if ( curAnimation.applyScale )
                anim->frameCache[0][i]->GetTransform()->SetScale(scale[i]);
        }
        return;
    }

    t.clear();
    r.clear();
    s.clear();
    std::vector<KG::Utill::HashString> nextIndexName;

    nextIndexName.resize(animSet->layers[0].nodeAnimations.size());
    t.resize(nextAnimations[0].index.size());
    r.resize(nextAnimations[0].index.size());
    s.resize(nextAnimations[0].index.size());

    animCount = 0;
    totalWeight = GetTotalWeight(0);
    for ( auto& idx : nextAnimations[0].index )
    {
        for ( auto& in : idx.second )
        {
            anim = &animations[idx.first][in.first];
            auto* inst = KG::Resource::ResourceContainer::GetInstance();
            float curT = anim->timer / anim->duration;
            anim->timer = curT * anim->duration;

            animSet = inst->LoadAnimation(anim->animationId, in.first);
            t[animCount].resize(animSet->layers[0].nodeAnimations.size());
            r[animCount].resize(animSet->layers[0].nodeAnimations.size());
            s[animCount].resize(animSet->layers[0].nodeAnimations.size());
            for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
            {
                nextIndexName[i] = animSet->layers[0].nodeAnimations[i].nodeId;
                if ( animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id) )
                {
                    continue;
                }

                t[animCount][i] = GetAnimationTranslation(animSet->layers[0].nodeAnimations[i], curT * anim->duration, anim->duration);
                r[animCount][i] = GetAnimationRotation(animSet->layers[0].nodeAnimations[i], curT * anim->duration, anim->duration);
                s[animCount][i] = GetAnimationScale(animSet->layers[0].nodeAnimations[i], curT * anim->duration, anim->duration);
            }
            animCount++;
        }
    }
    std::vector<XMFLOAT3> nextPos;
    std::vector<XMFLOAT4> nextRot;
    std::vector<XMFLOAT3> nextScale;
    nextPos.resize(animSet->layers[0].nodeAnimations.size());
    nextRot.resize(animSet->layers[0].nodeAnimations.size());
    nextScale.resize(animSet->layers[0].nodeAnimations.size());

    for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
    {
        if ( animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id) )
        {
            continue;
        }

        nextPos[i] = t[0][i];
        nextRot[i] = r[0][i];
        nextScale[i] = s[0][i];

        int count = 1;
        for ( auto iter = nextAnimations[0].index.begin(); iter != nextAnimations[0].index.end(); ++iter )
        {
            for ( auto it = iter->second.begin(); it != iter->second.end(); ++count )
            {
                float prevWeight = it++->second;
                if ( it == iter->second.end() )
                    break;
                float curWeight = it->second;
                float weight = curWeight / (curWeight + prevWeight);

                XMStoreFloat3(&nextPos[i], XMVectorLerp(XMLoadFloat3(&nextPos[i]), XMLoadFloat3(&t[count][i]), weight));
                XMStoreFloat4(&nextRot[i], XMQuaternionSlerp(XMLoadFloat4(&nextRot[i]), XMLoadFloat4(&r[count][i]), weight));
                XMStoreFloat3(&nextScale[i], XMVectorLerp(XMLoadFloat3(&nextScale[i]), XMLoadFloat3(&s[count][i]), weight));
            }
        }
    }
    for ( size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++ )
    {
        if ( animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id) )
        {
            continue;
        }

        XMFLOAT3 position = nextPos[i];
        for ( size_t p = 0; p < prevIndexName.size(); ++p )
        {
            if ( nextIndexName[i] == prevIndexName[p] )
            {
                position = pos[p];
                break;
            }
        }

        XMFLOAT4 rotation = nextRot[i];
        if ( changeIntercepted && !changeToDefault )
        {
            if ( prevFrameCache.size() >= i + 1 )
                rotation = prevFrameCache[i];
        }
        else
        {
            for ( size_t p = 0; p < prevIndexName.size(); ++p )
            {
                if ( nextIndexName[i] == prevIndexName[p] )
                {
                    rotation = rot[p];
                    break;
                }
            }
        }

        // XMFLOAT3 scaling = scale[i];
        XMFLOAT3 scaling = nextScale[i];
        for ( size_t p = 0; p < prevIndexName.size(); ++p )
        {
            if ( nextIndexName[i] == prevIndexName[p] )
            {
                scaling = scale[p];
                break;
            }
        }

        float weight = curAnimation.time / curAnimation.duration;

        XMStoreFloat3(&position, XMVectorLerp(XMLoadFloat3(&position), XMLoadFloat3(&nextPos[i]), weight));
        XMStoreFloat4(&rotation, XMQuaternionSlerp(XMLoadFloat4(&rotation), XMLoadFloat4(&nextRot[i]), weight));
        XMStoreFloat3(&scaling, XMVectorLerp(XMLoadFloat3(&scaling), XMLoadFloat3(&nextScale[i]), weight));

        if ( curAnimation.applyTransform && !this->isIgnoreTranslate )
            anim->frameCache[0][i]->GetTransform()->SetPosition(position);
        if ( curAnimation.applyRotation )
            anim->frameCache[0][i]->GetTransform()->SetRotation(rotation);
        if ( curAnimation.applyScale && !this->isIgnoreScale )
            anim->frameCache[0][i]->GetTransform()->SetScale(scaling);

    }

    curFrame = anim;
}

void KG::Component::FakeAnimationControllerComponent::Update(float elapsedTime)
{
    return;
    switch ( state )
    {
        case ANIMSTATE_PLAYING:
        case ANIMSTATE_STOP:
            PlayingUpdate(elapsedTime);
            break;
        case ANIMSTATE_CHANGING:
            ChangingUpdate(elapsedTime);
            break;
    }
}

void KG::Component::FakeAnimationControllerComponent::RegisterAnimation(const KG::Utill::HashString& animationId, UINT animationIndex)
{
    // if (animations.count(animationId.value) == 0) {
    if ( animations[animationId.value].count(animationIndex) == 0 )
    {
        auto* inst = KG::Resource::ResourceContainer::GetInstance();
        KG::Utill::AnimationSet* anim = inst->LoadAnimation(animationId, animationIndex);
        animations[animationId.value][animationIndex].animationId = animationId;
        if ( animations.size() <= 1 )
        {
            defaultAnimation.first = animationId;
            defaultAnimation.second = animationIndex;
        }
    }
    // }
}

// 애니메이션을 다 등록한 뒤에 사용할 것
void KG::Component::FakeAnimationControllerComponent::RegisterEvent(const KG::Utill::HashString& animationId, int keyFrame, const KG::Utill::HashString& eventId)
{
    /*
    if (GetAnimationIndex(animationId) < 0)
        return;
    if (events[animationId].size() > 0) {
        auto* inst = KG::Resource::ResourceContainer::GetInstance();
        Animation* anim = &animations[GetAnimationIndex(animationId)];
        KG::Utill::AnimationSet* animSet = inst->LoadAnimation(anim->animationId, 0);

        for (int i = 0; i < animSet->layers[0].nodeAnimations.size(); ++i) {
            if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
            {
                continue;
            }
            else {
                // 나중에 태형님이랑 상의해서 애니메이션 로드하는 부분에서 바꿔줘야 할듯??
                for (int key = 0; key < animSet->layers[0].nodeAnimations[i].rotation.x.size(); ++key) {
                    if (key == keyFrame) {
                        float time = animSet->layers[0].nodeAnimations[i].rotation.x[key].keyTime;
                        for (int j = 0; j < events[animationId].size(); ++j) {
                            if (events[animationId][j].eventId == eventId && events[animationId][j].time == time)
                                return;
                        }
                        events[animationId].emplace_back(AnimationEvent(eventId, time));
                    }
                }
            }
        }
    }
    else {
        auto* inst = KG::Resource::ResourceContainer::GetInstance();
        Animation* anim = &animations[GetAnimationIndex(animationId)];
        KG::Utill::AnimationSet* animSet = inst->LoadAnimation(anim->animationId, 0);

        for (int i = 0; i < animSet->layers[0].nodeAnimations.size(); ++i) {
            if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
            {
                continue;
            }
            else {
                for (int key = 0; key < animSet->layers[0].nodeAnimations[i].rotation.x.size(); ++key) {
                    if (key == keyFrame) {
                        float time = animSet->layers[0].nodeAnimations[i].rotation.x[key].keyTime;
                        events[animationId].emplace_back(AnimationEvent(eventId, time));
                    }
                }
            }
        }
    }
    */
}

void KG::Component::FakeAnimationControllerComponent::SetAnimation(const KG::Utill::HashString& animationId, UINT animationIndex, int repeat, float speed, bool clearNext, int weight)
{
    if ( clearNext )
        nextAnimations.clear();
    curAnimation.index.clear();
    curAnimation.index[animationId.value][animationIndex] = weight;
    // curAnimation.index[animationId.value].weight = weight;
    // curAnimation.index[animationId.value].animIndex = animationIndex;
    curAnimation.duration = GetDuration(animationId, animationIndex);
    if ( repeat == ANIMLOOP_INF )
        curAnimation.repeat = ANIMLOOP_INF;
    else
        curAnimation.repeat = repeat;
    if ( speed <= 0.f )
        speed = 0.01f;
    curAnimation.speed = speed;
    curAnimation.time = 0;

    //if (curAnimation.index.begin()->first != animationId.value)
    animations[animationId.value][animationIndex].timer = 0;

    state = ANIMSTATE_PLAYING;
    changeToDefault = false;
    changeIntercepted = false;
}

int  KG::Component::FakeAnimationControllerComponent::ChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex, int nextState, float blendingDuration, int repeat, bool addWeight, float speed)
{
    if ( blendingDuration <= 0 )
    {
        SetAnimation(animationId, animationIndex, repeat, speed);
        return ANIMINDEX_CURRENT;
    }
    else
    {
        if ( nextAnimations.size() > 0 )
        {
            if ( nextAnimations[0].index.count(animationId.value) != 0 && nextAnimations[0].index[animationId.value].count(animationIndex) != 0 )
                return ANIMINDEX_CHANGE;
            // if (nextAnimations[0].index[animationId.value] != NULL) {
            // nextAnimations[0].time = 0;
            // nextAnimations[0].duration = GetDuration(animationId, animationIndex);
            // if (repeat == ANIMLOOP_INF)
            // 	nextAnimations[0].repeat = ANIMLOOP_INF;
            // else
            // 	nextAnimations[0].repeat = repeat;
            // curAnimation.duration = blendingDuration;
            // state = ANIMSTATE_CHANGING;
            // return ANIMINDEX_CHANGE;
            //}
        }

        if ( state == ANIMSTATE_CHANGING )
        {
            changeIntercepted = true;
            prevFrameCache.clear();

            if ( curFrame )
            {
                for ( auto& frame : curFrame->frameCache[0] )
                {
                    if ( frame != nullptr )
                        prevFrameCache.emplace_back(frame->GetTransform()->GetRotation());
                }
            }
        }
        else
            changeIntercepted = false;
        nextAnimations.clear();
        state = ANIMSTATE_CHANGING;
        curAnimation.duration = blendingDuration;
        curAnimation.time = 0;
        AddNextAnimation(animationId, animationIndex, nextState, repeat);
        changeToDefault = false;
        return ANIMINDEX_CHANGE;
    }
}

void KG::Component::FakeAnimationControllerComponent::SetDefaultAnimation(KG::Utill::HashString defaultAnim, UINT animationIndex)
{
    if ( IsValidAnimationId(defaultAnim, animationIndex) )
    {
        defaultAnimation.first = defaultAnim;
        defaultAnimation.second = animationIndex;
    }
}


// return : next animation index
int KG::Component::FakeAnimationControllerComponent::AddNextAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex, int nextState, int repeat, float speed, int weight)
{
    if ( curAnimation.duration < 0 )
        return -1;
    AnimationCommand next;
    next.index[nextAnim.value][animationIndex] = weight;
    next.duration = GetDuration(nextAnim, animationIndex);
    if ( repeat == ANIMLOOP_INF )
        next.repeat = ANIMLOOP_INF;
    else
        next.repeat = repeat;
    next.time = 0;
    if ( speed <= 0.0f )
        speed = 0.01f;
    next.speed = speed;
    next.next = nextState;
    nextAnimations.emplace_back(next);

    return nextAnimations.size() - 1;
}


void KG::Component::FakeAnimationControllerComponent::BlendingAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex, float duration, int index, int weight)
{
    if ( !IsValidAnimationId(nextAnim) )
        return;
    if ( index == ANIMINDEX_CURRENT )
    {
        if ( duration > 0 )
        {
            curAnimation.duration = duration;
            curAnimation.time = 0;
        }
        if ( curAnimation.index.count(nextAnim.value) == 0 )
        {
            curAnimation.index[nextAnim.value][animationIndex] = 0;
        }
        else if ( curAnimation.index[nextAnim.value].count(animationIndex) == 0 )
        {
            curAnimation.index[nextAnim.value][animationIndex] = 0;
        }
        curAnimation.index[nextAnim.value][animationIndex] += 1;
    }
    else
    {
        if ( index >= nextAnimations.size() || index < 0 )
            return;
        if ( duration > 0 )
        {
            nextAnimations[index].duration = duration;
            nextAnimations[index].time = 0;
        }
        if ( nextAnimations[index].index.count(nextAnim.value) == 0 )
        {
            nextAnimations[index].index[nextAnim.value][animationIndex] = 0;
        }
        else if ( nextAnimations[index].index[nextAnim.value].count(animationIndex) == 0 )
        {
            nextAnimations[index].index[nextAnim.value][animationIndex] = 0;
        }
        nextAnimations[index].index[nextAnim.value][animationIndex] += 1;
    }
}

void KG::Component::FakeAnimationControllerComponent::SetAnimationWeight(int index, const KG::Utill::HashString& animationId, UINT animationIndex, int weight)
{
    if ( !IsValidAnimationId(animationId) )
        return;
    if ( index == ANIMINDEX_CURRENT )
    {
        if ( curAnimation.index.count(animationId.value) == 0 )
            curAnimation.index[animationId.value][animationIndex] = 0;
        else if ( curAnimation.index[animationId.value].count(animationIndex) == 0 )
            curAnimation.index[animationId.value][animationIndex] = 0;
        curAnimation.index[animationId.value][animationIndex] += 1;
    }
    else
    {
        if ( index < 0 || index >= nextAnimations.size() )
            return;
        if ( nextAnimations[index].index.count(animationId.value) == 0 )
            nextAnimations[index].index[animationId.value][animationIndex] = 0;
        else if ( nextAnimations[index].index[animationId.value].count(animationIndex) == 0 )
            nextAnimations[index].index[animationId.value][animationIndex] = 0;
        nextAnimations[index].index[animationId.value][animationIndex] += 1;
    }
}

void KG::Component::FakeAnimationControllerComponent::SetIgnoreScale(bool isUsing)
{
    this->isIgnoreScale = isUsing;
}

void KG::Component::FakeAnimationControllerComponent::SetIgnoreTranslate(bool isUsing)
{
    this->isIgnoreTranslate = isUsing;
}

bool KG::Component::FakeAnimationControllerComponent::OnDrawGUI()
{
    if ( ImGui::ComponentHeader<FakeAnimationControllerComponent>() )
    {
    }
    return false;
}

float KG::Component::FakeAnimationControllerComponent::GetDuration(const KG::Utill::HashString& animId, UINT animationIndex)
{
    if ( IsValidAnimationId(animId, animationIndex) )
        return animations[animId.value][animationIndex].duration;
    else
        return -1;
}

KG::Utill::HashString KG::Component::FakeAnimationControllerComponent::GetCurrentPlayingAnimationId() const
{
    return curAnimation.index.begin()->first;
}

UINT KG::Component::FakeAnimationControllerComponent::GetCurrentPlayingAnimationIndex() const
{
    return curAnimation.index.begin()->second.begin()->first;
}

float KG::Component::FakeAnimationControllerComponent::GetCurrentPlayingAnimationTime() const
{
    return curAnimation.time;
}

float KG::Component::FakeAnimationControllerComponent::GetCurrentPlayingAnimationDuration() const
{
    return curAnimation.duration;
}

//void KG::Component::FakeAnimationControllerComponent::SyncAnimation(const KG::Utill::HashString& anim, UINT animationIndex, float timer)
//{
//}

int KG::Component::FakeAnimationControllerComponent::ForceChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex, int nextState, float blendingDuration, int repeat, bool addWeight, float speed)
{
    if ( blendingDuration <= 0 )
    {
        SetAnimation(animationId, animationIndex, repeat, speed);
        return ANIMINDEX_CURRENT;
    }
    else
    {
        if ( nextAnimations.size() > 0 )
        {
            if ( nextAnimations[0].index.count(animationId.value) != 0 )
                return ANIMINDEX_CHANGE;
            // if (nextAnimations[0].index.count(animationId.value) == 0)
            // if (nextAnimations[0].index[animationId.value] != NULL) {
            nextAnimations[0].time = 0;
            nextAnimations[0].duration = GetDuration(animationId, animationIndex);
            if ( repeat == ANIMLOOP_INF )
                nextAnimations[0].repeat = ANIMLOOP_INF;
            else
                nextAnimations[0].repeat = GetDuration(animationId, animationIndex) * repeat;
            curAnimation.duration = blendingDuration;
            state = ANIMSTATE_CHANGING;
            return ANIMINDEX_CHANGE;
            //}
        }

        if ( state == ANIMSTATE_CHANGING )
        {
            changeIntercepted = true;
            prevFrameCache.clear();

            if ( curFrame )
            {
                for ( auto& frame : curFrame->frameCache[0] )
                {
                    prevFrameCache.emplace_back(frame->GetTransform()->GetRotation());
                }
            }
        }
        else
            changeIntercepted = false;
        nextAnimations.clear();
        state = ANIMSTATE_CHANGING;
        curAnimation.duration = blendingDuration;
        curAnimation.time = 0;
        auto pc = curAnimation.index.begin();
        if ( pc->first != animationId.value && pc->second.begin()->first != animationIndex )
            animations[pc->first][pc->second.begin()->first].timer = 0;
        AddNextAnimation(animationId, animationIndex, nextState, repeat);
        return ANIMINDEX_CHANGE;

    }
}