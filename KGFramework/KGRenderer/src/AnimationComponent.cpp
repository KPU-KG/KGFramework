#include "pch.h"
#include "fbxImpoter.h"
#include "GameObject.h"
#include "KGGeometry.h"
#include "MathHelper.h"
#include "ResourceContainer.h"

#include "Scene.h"
#include "AnimationComponent.h"
#include "GeometryComponent.h"

#include <iostream>

using namespace DirectX;

void KG::Component::BoneTransformComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
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

KG::Component::BoneTransformComponent::BoneTransformComponent()
	: rootNodeIdProp("RootNodeID", rootNodeTag)
{
}

KG::Core::GameObject* KG::Component::BoneTransformComponent::BoneIndexToGameObject( UINT index, UINT submeshIndex ) const
{
	return this->frameCache[submeshIndex][index];
}

void KG::Component::BoneTransformComponent::SetRootNode(KG::Core::GameObject* object)
{
	rootNode = object;
	this->rootNodeTag = rootNode->tag;
}

void KG::Component::BoneTransformComponent::InitializeBone( KG::Core::GameObject* rootNode )
{
	this->geometry = gameObject->GetComponent<KG::Component::GeometryComponent>();

	this->frameCache.resize( this->geometry->geometrys.size() );
	for ( size_t i = 0; i < this->geometry->geometrys.size(); i++ )
	{
		auto& boneIds = this->geometry->geometrys[i]->boneIds;
		auto& cache = this->frameCache[i];
		cache.resize( boneIds.size() );
		rootNode->MatchBoneToObject( boneIds, cache );
#ifdef _DEBUG
		for ( auto& i : cache )
		{
			if ( i == nullptr )
			{
				DebugErrorMessage( "Bone Not Linked Object!" );
			}
		}
#endif
	}
}

void KG::Component::BoneTransformComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->rootNodeIdProp.OnDataLoad(componentElement);
}

void KG::Component::BoneTransformComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::BoneTransformComponent);
	this->rootNodeIdProp.OnDataSave(componentElement);
}

bool KG::Component::BoneTransformComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<BoneTransformComponent>() )
	{
		this->rootNodeIdProp.OnDrawGUI();
	}
	return false;
}


static float GetTimeData(const std::vector<KG::Utill::KeyData>& data, float currentTime, float duration, float defaultValue = 0.0f)
{
	float value = defaultValue;
	if (!data.empty())
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

		if (prev == data.end())
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

		if (last == data.end())
		{
			keyTime1 = duration;
			keyValue1 = value;
		}
		else
		{
			while (last != data.end() && keyTime1 <= keyTime0) {
				keyTime1 = last->keyTime;
				keyValue1 = last->value;
				last++;
			}
		}
		if (keyTime0 == keyTime1)
			return keyValue1;
		value = KG::Math::Lerp(keyValue0, keyValue1, abs(currentTime - keyTime0) / abs(keyTime1 - keyTime0));
		//value = keyValue0;
	}
	return value;
}

static XMFLOAT3 GetAnimationTranslation(const KG::Utill::NodeAnimation& anim, float currentTime, float duration)
{
	XMFLOAT3 t = {};
	if (anim.translation.x.size() <= 0 && anim.translation.y.size() <= 0 && anim.translation.z.size() <= 0) {
		t = { 0,0,0 };
	}
	else {
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

void KG::Component::Animation::MatchNode(KG::Core::GameObject* gameObject)
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	KG::Utill::AnimationSet* anim = inst->LoadAnimation(animationId, 0);
	for (auto& layer : anim->layers)
	{
		auto& cache = this->frameCache.emplace_back();
		std::vector<KG::Utill::HashString> ids;
		cache.resize(layer.nodeAnimations.size());
		for (auto& n : layer.nodeAnimations)
		{
			ids.push_back(n.nodeId);
		}
		gameObject->MatchBoneToObject(ids, cache);
	}

}

void KG::Component::Animation::SetDuration(KG::Utill::AnimationSet* anim)
{
	for (auto& a : anim->layers[0].nodeAnimations)
	{
		if (!a.translation.x.empty()) this->duration = std::max(this->duration, a.translation.x.back().keyTime);
		if (!a.translation.y.empty()) this->duration = std::max(this->duration, a.translation.y.back().keyTime);
		if (!a.translation.z.empty()) this->duration = std::max(this->duration, a.translation.z.back().keyTime);
		if (!a.rotation.x.empty()) this->duration = std::max(this->duration, a.rotation.x.back().keyTime);
		if (!a.rotation.y.empty()) this->duration = std::max(this->duration, a.rotation.y.back().keyTime);
		if (!a.rotation.z.empty()) this->duration = std::max(this->duration, a.rotation.z.back().keyTime);
		if (!a.scale.x.empty()) this->duration = std::max(this->duration, a.scale.x.back().keyTime);
		if (!a.scale.y.empty()) this->duration = std::max(this->duration, a.scale.y.back().keyTime);
		if (!a.scale.z.empty()) this->duration = std::max(this->duration, a.scale.z.back().keyTime);
	}
}

void KG::Component::Animation::Initialize(KG::Core::GameObject* gameObject)
{
	MatchNode(gameObject);
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	KG::Utill::AnimationSet* anim = inst->LoadAnimation(animationId, 0);
	SetDuration(anim);
}

int KG::Component::AnimationControllerComponent::GetTotalWeight(int index)
{
	if (index == -1) {
		int total = 0;
		for (auto& w : curAnimation.index)
			total += w.second;
		return total;
	}
	else if (index < nextAnimations.size()) {
		int total = 0;
		for (auto& w : nextAnimations[index].index) {
			total += w.second;
		}
		return total;
	}
	return 1;
}


void KG::Component::AnimationControllerComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	for (auto& animation : animations) {
		animation.second.Initialize(this->gameObject);
		// animation.Initialize(this->gameObject);
	}
}

bool KG::Component::AnimationControllerComponent::IsValidAnimationId(const KG::Utill::HashString& animationId)
{
	if (animations[animationId.value].isRegistered) {
		return true;
	}
	else {
		animations.erase(animationId.value);
		return false;
	}
}

void KG::Component::AnimationControllerComponent::OnDestroy()
{

}

void KG::Component::AnimationControllerComponent::PlayingUpdate(float elapsedTime)
{
	if (curAnimation.index.size() <= 0)
		return;
	Animation* anim = &animations[curAnimation.index.begin()->first];
	// curAnimation.index.
	

	anim->timer += elapsedTime * curAnimation.speed;

	if (anim->timer >= anim->duration) {
		anim->timer -= anim->duration;

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



	float T = anim->timer / anim->duration;
	curAnimation.time += elapsedTime;

	KG::Utill::AnimationSet* animSet = nullptr;

	std::vector<std::vector<DirectX::XMFLOAT3>> t;
	std::vector<std::vector<DirectX::XMFLOAT4>> r;
	std::vector<std::vector<DirectX::XMFLOAT3>> s;
	t.resize(curAnimation.index.size());
	r.resize(curAnimation.index.size());
	s.resize(curAnimation.index.size());

	int animCount = 0;
	int totalWeight = GetTotalWeight(ANIMINDEX_CURRENT);
	for (auto& idx : curAnimation.index) {
		anim = &animations[idx.first];
		auto* inst = KG::Resource::ResourceContainer::GetInstance();

		anim->timer = T * anim->duration;

		animSet = inst->LoadAnimation(anim->animationId, 0);
		t[animCount].resize(animSet->layers[0].nodeAnimations.size());
		r[animCount].resize(animSet->layers[0].nodeAnimations.size());
		s[animCount].resize(animSet->layers[0].nodeAnimations.size());
		for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++)
		{
			if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
			{
				continue;
			}

			t[animCount][i] = GetAnimationTranslation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
			r[animCount][i] = GetAnimationRotation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
			s[animCount][i] = GetAnimationScale(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
		}
		animCount++;
	}

	for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++)
	{
		if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
		{
			continue;
		}

		XMFLOAT3 pos = t[0][i];
		XMFLOAT4 rot = r[0][i];
		XMFLOAT3 scale = s[0][i];

		int count = 1;
		for (auto iter = curAnimation.index.begin(); iter != curAnimation.index.end(); count++) {
			float prevWeight = iter++->second;
			if (iter == curAnimation.index.end())
				break;
			float curWeight = iter->second;
			float weight = curWeight / (curWeight + prevWeight);

			XMStoreFloat3(&pos, XMVectorLerp(XMLoadFloat3(&pos), XMLoadFloat3(&t[count][i]), weight));
			XMStoreFloat4(&rot, XMQuaternionSlerp(XMLoadFloat4(&rot), XMLoadFloat4(&r[count][i]), weight));
			XMStoreFloat3(&scale, XMVectorLerp(XMLoadFloat3(&scale), XMLoadFloat3(&s[count][i]), weight));
		}

		if ( anim->frameCache[0][i] != nullptr )
		{
			if (curAnimation.applyTransform && !this->isIgnoreTranslate )
				anim->frameCache[0][i]->GetTransform()->SetPosition(pos);
			if (curAnimation.applyRotation)
				anim->frameCache[0][i]->GetTransform()->SetRotation(rot);
			if (curAnimation.applyScale && !this->isIgnoreScale)
				anim->frameCache[0][i]->GetTransform()->SetScale(scale);
		}
	}

	curFrame = anim;

	if (curAnimation.duration < 0)
		; // loop inf
	else if (curAnimation.time >= curAnimation.duration) {
		float animT = animations[curAnimation.index.begin()->first].timer / animations[curAnimation.index.begin()->first].duration;
		if (nextAnimations.size() <= 0) {
			animations[defaultAnimation.value].timer = animT * animations[defaultAnimation.value].duration;
			ChangeAnimation(defaultAnimation, 0.5f, -1);
		}
		else {
			animations[nextAnimations[0].index.begin()->first].timer = animT * animations[nextAnimations[0].index.begin()->first].duration;
			curAnimation = nextAnimations[0];
			nextAnimations.erase(nextAnimations.begin());
		}
		state = curAnimation.next;
	}
}

void KG::Component::AnimationControllerComponent::ChangingUpdate(float elapsedTime)
{
	if (curAnimation.index.size() <= 0)
		return;
	Animation* anim = &animations[curAnimation.index.begin()->first];
	// curAnimation.index.


	anim->timer += elapsedTime * curAnimation.speed;

	if (anim->timer >= anim->duration) {
		anim->timer -= anim->duration;

		// 한 애니메이션 넘어가면 이벤트 초기화
		// for (int idx : curAnimation.index) {
		// 	if (events[animations[idx].animationId].size() > 0) {
		// 		for (auto& e : events[animations[idx].animationId]) {
		// 			e.activated = false;
		// 		}
		// 	}
		// }
	}

	float T = anim->timer / anim->duration;

	curAnimation.time += elapsedTime;

	KG::Utill::AnimationSet* animSet = nullptr;

	std::vector<std::vector<DirectX::XMFLOAT3>> t;
	std::vector<std::vector<DirectX::XMFLOAT4>> r;
	std::vector<std::vector<DirectX::XMFLOAT3>> s;
	t.resize(curAnimation.index.size());
	r.resize(curAnimation.index.size());
	s.resize(curAnimation.index.size());

	int animCount = 0;
	int totalWeight = GetTotalWeight(ANIMINDEX_CURRENT);
	for (auto& idx : curAnimation.index) {
		anim = &animations[idx.first];
		auto* inst = KG::Resource::ResourceContainer::GetInstance();

		anim->timer = T * anim->duration;

		animSet = inst->LoadAnimation(anim->animationId, 0);
		t[animCount].resize(animSet->layers[0].nodeAnimations.size());
		r[animCount].resize(animSet->layers[0].nodeAnimations.size());
		s[animCount].resize(animSet->layers[0].nodeAnimations.size());
		for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++)
		{
			if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
			{
				continue;
			}

			t[animCount][i] = GetAnimationTranslation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
			r[animCount][i] = GetAnimationRotation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
			s[animCount][i] = GetAnimationScale(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
		}
		animCount++;
	}

	std::vector<XMFLOAT3> pos;
	std::vector<XMFLOAT4> rot;
	std::vector<XMFLOAT3> scale;
	pos.resize(animSet->layers[0].nodeAnimations.size());
	rot.resize(animSet->layers[0].nodeAnimations.size());
	scale.resize(animSet->layers[0].nodeAnimations.size());

	for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++)
	{
		if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
		{
			continue;
		}

		pos[i] = t[0][i];
		rot[i] = r[0][i];
		scale[i] = s[0][i];

		int count = 1;
		for (auto iter = curAnimation.index.begin(); iter != curAnimation.index.end(); count++) {
			float prevWeight = iter++->second;
			if (iter == curAnimation.index.end())
				break;
			float curWeight = iter->second;
			float weight = curWeight / (curWeight + prevWeight);

			XMStoreFloat4(&rot[i], XMQuaternionSlerp(XMLoadFloat4(&rot[i]), XMLoadFloat4(&r[count][i]), weight));
			XMStoreFloat3(&scale[i], XMVectorLerp(XMLoadFloat3(&scale[i]), XMLoadFloat3(&s[count][i]), weight));
		}

	}

	/// next animation 계산 (blending)

	if (nextAnimations.size() <= 0) {
		for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++) {
			if (curAnimation.applyTransform)
				;
			if (curAnimation.applyRotation)
				anim->frameCache[0][i]->GetTransform()->SetRotation(rot[i]);
			if (curAnimation.applyScale)
				anim->frameCache[0][i]->GetTransform()->SetScale(scale[i]);
		}
		return;
	}

	t.clear();
	r.clear();
	s.clear();

	t.resize(nextAnimations[0].index.size());
	r.resize(nextAnimations[0].index.size());
	s.resize(nextAnimations[0].index.size());

	animCount = 0;
	totalWeight = GetTotalWeight(0);
	for (auto& idx : nextAnimations[0].index) {
		anim = &animations[idx.first];
		auto* inst = KG::Resource::ResourceContainer::GetInstance();

		anim->timer = T * anim->duration;

		animSet = inst->LoadAnimation(anim->animationId, 0);
		t[animCount].resize(animSet->layers[0].nodeAnimations.size());
		r[animCount].resize(animSet->layers[0].nodeAnimations.size());
		s[animCount].resize(animSet->layers[0].nodeAnimations.size());
		for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++)
		{
			if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
			{
				continue;
			}

			t[animCount][i] = GetAnimationTranslation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
			r[animCount][i] = GetAnimationRotation(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
			s[animCount][i] = GetAnimationScale(animSet->layers[0].nodeAnimations[i], T * anim->duration, anim->duration);
		}
		animCount++;
	}

	std::vector<XMFLOAT3> nextPos;
	std::vector<XMFLOAT4> nextRot;
	std::vector<XMFLOAT3> nextScale;
	nextPos.resize(animSet->layers[0].nodeAnimations.size());
	nextRot.resize(animSet->layers[0].nodeAnimations.size());
	nextScale.resize(animSet->layers[0].nodeAnimations.size());

	for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++)
	{
		if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
		{
			continue;
		}

		nextPos[i] = t[0][i];
		nextRot[i] = r[0][i];
		nextScale[i] = s[0][i];

		int count = 1;
		for (auto iter = nextAnimations[0].index.begin(); iter != nextAnimations[0].index.end(); count++) {
			float prevWeight = iter++->second;
			if (iter == nextAnimations[0].index.end())
				break;
			float curWeight = iter->second;
			float weight = curWeight / (curWeight + prevWeight);

			XMStoreFloat4(&nextRot[i], XMQuaternionSlerp(XMLoadFloat4(&nextRot[i]), XMLoadFloat4(&r[count][i]), weight));
			XMStoreFloat3(&nextScale[i], XMVectorLerp(XMLoadFloat3(&nextScale[i]), XMLoadFloat3(&s[count][i]), weight));
		}
	}

	for (size_t i = 0; i < animSet->layers[0].nodeAnimations.size(); i++)
	{
		if (animSet->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString("RootNode"_id))
		{
			continue;
		}

		XMFLOAT3 position = pos[i];
		XMFLOAT4 rotation;
		if (changeIntercepted)
			rotation = prevFrameCache[i];
		else
			rotation = rot[i];
		XMFLOAT3 scaling = scale[i];

		float weight = curAnimation.time / curAnimation.duration;

		XMStoreFloat4(&rotation, XMQuaternionSlerp(XMLoadFloat4(&rotation), XMLoadFloat4(&nextRot[i]), weight));
		XMStoreFloat3(&scaling, XMVectorLerp(XMLoadFloat3(&scaling), XMLoadFloat3(&nextScale[i]), weight));

		if (curAnimation.applyTransform && !this->isIgnoreTranslate )
			anim->frameCache[0][i]->GetTransform()->SetPosition(position);
		if (curAnimation.applyRotation)
			anim->frameCache[0][i]->GetTransform()->SetRotation(rotation);
		if (curAnimation.applyScale && !this->isIgnoreScale )
			anim->frameCache[0][i]->GetTransform()->SetScale(scaling);

	}

	curFrame = anim;

	if (curAnimation.duration < 0)
		; // loop inf
	else if (curAnimation.time >= curAnimation.duration) {
		float animT = animations[curAnimation.index.begin()->first].timer / animations[curAnimation.index.begin()->first].duration;
		if (nextAnimations.size() <= 0) {
			animations[defaultAnimation.value].timer = animT * animations[defaultAnimation.value].duration;
			ChangeAnimation(defaultAnimation, 0.5f, -1);
		}
		else {
			animations[nextAnimations[0].index.begin()->first].timer = animT * animations[nextAnimations[0].index.begin()->first].duration;
			curAnimation = nextAnimations[0];
			nextAnimations.erase(nextAnimations.begin());
		}
		state = curAnimation.next;
	}
}

void KG::Component::AnimationControllerComponent::Update(float elapsedTime)
{
	DebugNormalMessage("Update Animation");
	switch (state) {
	case ANIMSTATE_PLAYING:
		PlayingUpdate(elapsedTime);
		break;
	case ANIMSTATE_CHANGING:
		ChangingUpdate(elapsedTime);
		break;
	}
}

void KG::Component::AnimationControllerComponent::RegisterAnimation(const KG::Utill::HashString& animationId, UINT animationIndex)
{
	if (!animations[animationId.value].isRegistered) {
		auto* inst = KG::Resource::ResourceContainer::GetInstance();
		KG::Utill::AnimationSet* anim = inst->LoadAnimation(animationId, animationIndex);
		animations[animationId.value].animationId = animationId;
		animations[animationId.value].isRegistered = true;
		if (animations.size() <= 1)
			defaultAnimation = animationId;
	}
	else {
		animations.erase(animationId.value);
	}
}

// 애니메이션을 다 등록한 뒤에 사용할 것
void KG::Component::AnimationControllerComponent::RegisterEvent(const KG::Utill::HashString& animationId, int keyFrame, const KG::Utill::HashString& eventId)
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

void KG::Component::AnimationControllerComponent::SetAnimation(const KG::Utill::HashString& animationId, int repeat, float speed, bool clearNext, int weight) {
	if (clearNext)
		nextAnimations.clear();
	curAnimation.index.clear();
	// curAnimation.index.push_back(GetAnimationIndex(animationId));
	// curAnimation.weight.clear();
	// curAnimation.weight.push_back(weight);
	curAnimation.index[animationId.value] = weight;
	if (repeat == ANIMLOOP_INF)
		curAnimation.duration = ANIMLOOP_INF;
	else
		curAnimation.duration = GetDuration(animationId) * repeat;
	curAnimation.time = 0;
	if (speed <= 0.f)
		speed = 0.01f;
	curAnimation.speed = speed;
}

int  KG::Component::AnimationControllerComponent::ChangeAnimation(const KG::Utill::HashString& animationId, int nextState, float blendingDuration, int repeat, bool addWeight, float speed)
{
	if (blendingDuration <= 0) {
		SetAnimation(animationId, repeat, speed);
		return ANIMINDEX_CURRENT;
	}
	else {
		if (nextAnimations.size() > 0) {
			// IsValidAnimationId
			if (nextAnimations[0].index[animationId.value] != NULL) {
				nextAnimations[0].time = 0;
				if (repeat == ANIMLOOP_INF)
					nextAnimations[0].duration = ANIMLOOP_INF;
				else	
					nextAnimations[0].duration = GetDuration(animationId) * repeat;
				curAnimation.duration = blendingDuration;
				state = ANIMSTATE_CHANGING;
				return ANIMINDEX_CHANGE;
			}
		}

		if (state == ANIMSTATE_CHANGING) {
			changeIntercepted = true;
			prevFrameCache.clear();

			// Update에서 마지막으로 사용한 anim
			// 요거는 변수 하나 저장해놓는게 좋을듯!
			// Animation* anim = &animations[nextAnimations[0].index[nextAnimations[0].index.size() - 1]];
			// for (int i = 0; i < anim->frameCache[0].size(); ++i) {
			// 	prevFrameCache.emplace_back(anim->frameCache[0][i]->GetTransform()->GetRotation());
			// }

			if (curFrame) {
				for (auto& frame : curFrame->frameCache[0]) {
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
		AddNextAnimation(animationId, nextState, repeat);
		return ANIMINDEX_CHANGE;
	}
}

void KG::Component::AnimationControllerComponent::SetDefaultAnimation(KG::Utill::HashString defaultAnim)
{
	if (IsValidAnimationId(defaultAnim))
		defaultAnimation = defaultAnim;
}

// return : next animation index
int KG::Component::AnimationControllerComponent::AddNextAnimation(const KG::Utill::HashString& nextAnim, int nextState, int repeat, float speed, int weight)
{
	if (curAnimation.duration < 0)
		return -1;
	AnimationCommand next;
	next.index[nextAnim.value] = weight;
	if (repeat == ANIMLOOP_INF)
		next.duration = ANIMLOOP_INF;
	else
		next.duration = GetDuration(nextAnim) * repeat;
	next.time = 0;
	if (speed <= 0.0f)
		speed = 0.01f;
	next.speed = speed;
	next.next = nextState;
	nextAnimations.emplace_back(next);

	return nextAnimations.size() - 1;
}


void KG::Component::AnimationControllerComponent::BlendingAnimation(const KG::Utill::HashString& nextAnim, float duration, int index, int weight)
{
	if (!IsValidAnimationId(nextAnim))
		return;
	if (index == ANIMINDEX_CURRENT) {
		if (duration > 0) {
			curAnimation.duration = duration;
			curAnimation.time = 0;
		}
		if (curAnimation.index[nextAnim.value] == NULL) {
			curAnimation.index[nextAnim.value] = 0;
		}
		curAnimation.index[nextAnim.value] += 1;
	}
	else {
		if (index >= nextAnimations.size() || index < 0)
			return;
		if (duration > 0) {
			nextAnimations[index].duration = duration;
			nextAnimations[index].time = 0;
		}
		if (nextAnimations[index].index[nextAnim.value] == NULL) {
			nextAnimations[index].index[nextAnim.value] = 0;
		}
		nextAnimations[index].index[nextAnim.value] += 1;
	}
}

void KG::Component::AnimationControllerComponent::SetAnimationWeight(int index, const KG::Utill::HashString& animationId, int weight)
{
	if (!IsValidAnimationId(animationId))
		return;
	if (index == ANIMINDEX_CURRENT) {
		if (curAnimation.index[animationId.value] == NULL)
			curAnimation.index[animationId.value] = 0;
		curAnimation.index[animationId.value] += 1;
	}
	else {
		if (index < 0 || index >= nextAnimations.size())
			return;
		if (nextAnimations[index].index[animationId.value] == NULL)
			nextAnimations[index].index[animationId.value] = 0;
		nextAnimations[index].index[animationId.value] += 1;
	}
}

void KG::Component::AnimationControllerComponent::SetIgnoreScale(bool isUsing)
{
	this->isIgnoreScale = isUsing;
}

void KG::Component::AnimationControllerComponent::SetIgnoreTranslate(bool isUsing)
{
	this->isIgnoreTranslate = isUsing;
}

bool KG::Component::AnimationControllerComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<AnimationControllerComponent>() )
	{
	}
	return false;
}
