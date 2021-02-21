#include "pch.h"
#include "fbxImpoter.h"
#include "GameObject.h"
#include "KGGeometry.h"
#include "MathHelper.h"
#include "ResourceContainer.h"

#include "AnimationComponent.h"
#include "GeometryComponent.h"

using namespace DirectX;

void KG::Component::BoneTransformComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );

}

KG::Core::GameObject* KG::Component::BoneTransformComponent::BoneIndexToGameObject( UINT index, UINT submeshIndex ) const
{
	return this->frameCache[submeshIndex][index];
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
			keyTime1 = last->keyTime;
			keyValue1 = last->value;
		}
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
	r.x = XMConvertToRadians(GetTimeData(anim.rotation.x, currentTime, duration));
	r.y = XMConvertToRadians(GetTimeData(anim.rotation.y, currentTime, duration));
	r.z = XMConvertToRadians(GetTimeData(anim.rotation.z, currentTime, duration));
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

// duration 구하는 함수도 새로 만들어야 함
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

int KG::Component::AnimationControllerComponent::GetAnimationIndex(const KG::Utill::HashString& animationId)
{
	for (int i = 0; i < animations.size(); ++i) {
		if (animations[i].animationId == animationId)
			return i;
	}
	return -1;
}

int KG::Component::AnimationControllerComponent::GetAnimationCommandIndex(const KG::Utill::HashString animationId, int index)
{
	int animIdx = GetAnimationIndex(animationId);
	if (index == -1) {
		for (int i = 0; i < curAnimation.index.size(); ++i) {
			if (curAnimation.index[i] == animIdx)
				return i;
		}
	}
	else if (index < nextAnimations.size()) {
		for (int i = 0; i < nextAnimations[index].index.size(); ++i) {
			if (nextAnimations[index].index[i] == animIdx)
				return i;
		}
	}
	return -1;
}

int KG::Component::AnimationControllerComponent::GetTotalWeight(int index)
{
	if (index == -1) {
		int total = 0;
		for (auto& w : curAnimation.weight)
			total += w;
		return total;
	}
	else if (index < nextAnimations.size()) {
		int total = 0;
		for (auto& w : nextAnimations[index].weight) {
			total += w;
		}
		return total;
	}
	return 1;
}


void KG::Component::AnimationControllerComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	for (auto& animation : animations) {
		animation.Initialize(this->gameObject);
	}
}

void KG::Component::AnimationControllerComponent::OnDestroy()
{

}

void KG::Component::AnimationControllerComponent::PlayingUpdate(float elapsedTime)
{
	if (curAnimation.index.size() <= 0 || curAnimation.index[0] == -1)
		return;
	Animation* anim = &animations[curAnimation.index[0]];
	anim->timer += elapsedTime * curAnimation.speed;

	if (anim->timer >= anim->duration) {
		anim->timer -= anim->duration;
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
	int totalWeight = GetTotalWeight(-1);
	for (auto& idx : curAnimation.index) {
		anim = &animations[idx];
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

		for (int count = 1; count < curAnimation.index.size(); ++count) {
			float weight = (float)curAnimation.weight[count] / (float)(curAnimation.weight[count - 1] + curAnimation.weight[count]);
			XMStoreFloat3(&pos, XMVectorLerp(XMLoadFloat3(&pos), XMLoadFloat3(&t[count][i]), weight));
			XMStoreFloat4(&rot, XMQuaternionSlerp(XMLoadFloat4(&rot), XMLoadFloat4(&r[count][i]), weight));
			XMStoreFloat3(&scale, XMVectorLerp(XMLoadFloat3(&scale), XMLoadFloat3(&s[count][i]), weight));
		}
		if (curAnimation.applyTransform)
			; // anim->frameCache[0][i]->GetTransform()->SetPosition(pos);
		if (curAnimation.applyRotation)
			anim->frameCache[0][i]->GetTransform()->SetRotation(rot);
		if (curAnimation.applyScale)
			anim->frameCache[0][i]->GetTransform()->SetScale(scale);
	}

	if (curAnimation.duration < 0)
		; // loop inf
	else if (curAnimation.time >= curAnimation.duration) {
		float animT = animations[curAnimation.index[0]].timer / animations[curAnimation.index[0]].duration;
		if (nextAnimations.size() <= 0) {
			animations[GetAnimationIndex(defaultAnimation)].timer = animT * animations[GetAnimationIndex(defaultAnimation)].duration;
			ChangeAnimation(defaultAnimation, 0.1f, -1);
		}
		else {
			animations[nextAnimations[0].index[0]].timer = animT * animations[nextAnimations[0].index[0]].duration;
			curAnimation = nextAnimations[0];
			nextAnimations.erase(nextAnimations.begin());
		}
		state = curAnimation.next;
	}
}

void KG::Component::AnimationControllerComponent::ChangingUpdate(float elapsedTime)
{
	if (curAnimation.index.size() <= 0 || curAnimation.index[0] == -1)
		return;
	Animation* anim = &animations[curAnimation.index[0]];
	anim->timer += elapsedTime * curAnimation.speed;

	if (anim->timer >= anim->duration) {
		anim->timer -= anim->duration;
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
	int totalWeight = GetTotalWeight(-1);
	for (auto& idx : curAnimation.index) {
		anim = &animations[idx];
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

		for (int count = 1; count < curAnimation.index.size(); ++count) {
			float weight = (float)curAnimation.weight[count - 1] / (float)(curAnimation.weight[count - 1] + curAnimation.weight[count]);
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
		anim = &animations[idx];
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

		for (int count = 1; count < nextAnimations[0].index.size(); ++count) {
			float weight = (float)nextAnimations[0].weight[count - 1] / (float)(nextAnimations[0].weight[count - 1] + nextAnimations[0].weight[count]);
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
		XMFLOAT4 rotation = rot[i];
		XMFLOAT3 scaling = scale[i];

		float weight = curAnimation.time / curAnimation.duration;
		XMStoreFloat4(&rotation, XMQuaternionSlerp(XMLoadFloat4(&rotation), XMLoadFloat4(&nextRot[i]), weight));
		XMStoreFloat3(&scaling, XMVectorLerp(XMLoadFloat3(&scaling), XMLoadFloat3(&nextScale[i]), weight));

		if (curAnimation.applyTransform)
			;
		if (curAnimation.applyRotation)
			anim->frameCache[0][i]->GetTransform()->SetRotation(rotation);
		if (curAnimation.applyScale)
			anim->frameCache[0][i]->GetTransform()->SetScale(scaling);
	}

	if (curAnimation.duration < 0)
		; // loop inf
	else if (curAnimation.time >= curAnimation.duration) {
		float animT = animations[curAnimation.index[0]].timer / animations[curAnimation.index[0]].duration;
		if (nextAnimations.size() <= 0) {
			animations[GetAnimationIndex(defaultAnimation)].timer = animT * animations[GetAnimationIndex(defaultAnimation)].duration;
			ChangeAnimation(defaultAnimation, 0.1f, -1);
		}
		else {
			animations[nextAnimations[0].index[0]].timer = animT * animations[nextAnimations[0].index[0]].duration;
			curAnimation = nextAnimations[0];
			nextAnimations.erase(nextAnimations.begin());
		}
		state = curAnimation.next;
	}
}

void KG::Component::AnimationControllerComponent::Update(float elapsedTime)
{
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
	if (GetAnimationIndex(animationId) == -1) {
		auto* inst = KG::Resource::ResourceContainer::GetInstance();
		KG::Utill::AnimationSet* anim = inst->LoadAnimation(animationId, animationIndex);
		Animation a;
		a.animationId = animationId;
		if (animations.size() <= 0)
			defaultAnimation = animationId;
		animations.emplace_back(a);
	}
}

void KG::Component::AnimationControllerComponent::SetAnimation(const KG::Utill::HashString& animationId, float duration, float speed, bool clearNext, int weight) {
	if (clearNext)
		nextAnimations.clear();
	curAnimation.index.clear();
	curAnimation.index.push_back(GetAnimationIndex(animationId));
	curAnimation.weight.clear();
	curAnimation.weight.push_back(weight);
	curAnimation.duration = duration;
	curAnimation.time = 0;
	if (speed <= 0.f)
		speed = 0.01f;
	curAnimation.speed = speed;
}

int KG::Component::AnimationControllerComponent::ChangeAnimation(const KG::Utill::HashString& animationId, int nextState, float blendingDuration, float animationDuration, float speed)
{
	if (blendingDuration <= 0) {
		SetAnimation(animationId, animationDuration, speed);
		return ANIMINDEX_CURRENT;
	}
	else {
		nextAnimations.clear();
		state = ANIMSTATE_CHANGING;
		// BlendingAnimation(animationId, blendingDuration);
		curAnimation.duration = blendingDuration;
		curAnimation.time = 0;
		AddNextAnimation(animationId, nextState, animationDuration);
		return ANIMINDEX_CHANGE;
	}
}

void KG::Component::AnimationControllerComponent::SetDefaultAnimation(KG::Utill::HashString defaultAnim)
{
	if (GetAnimationIndex(defaultAnim) >= 0) {
		defaultAnimation = defaultAnim;
	}
}

// return : next animation index
int KG::Component::AnimationControllerComponent::AddNextAnimation(const KG::Utill::HashString nextAnim, int nextState, float duration, float speed, int weight)
{
	if (curAnimation.duration < 0)
		return -1;
	AnimationCommand next;
	next.index.push_back(GetAnimationIndex(nextAnim));
	next.weight.push_back(weight);
	next.duration = duration;
	next.time = 0;
	if (speed <= 0.0f)
		speed = 0.01f;
	next.speed = speed;
	next.next = nextState;
	nextAnimations.push_back(next);

	return nextAnimations.size() - 1;
}


void KG::Component::AnimationControllerComponent::BlendingAnimation(const KG::Utill::HashString nextAnim, float duration, int index, int weight)
{
	int idx = GetAnimationIndex(nextAnim);
	if (idx == -1)
		return;
	if (index == -1) {
		if (duration > 0) {
			curAnimation.duration = duration;
			curAnimation.time = 0;
		}
		for (int i = 0; i < curAnimation.index.size(); ++i) {
			if (curAnimation.index[i] == idx) {
				curAnimation.weight[i] += 1;
				return;
			}
		}
		curAnimation.index.push_back(GetAnimationIndex(nextAnim));
		curAnimation.weight.push_back(weight);
	}
	else {
		if (index < 0 || index >= nextAnimations.size())
			return;
		if (duration > 0) {
			nextAnimations[index].duration = duration;
			nextAnimations[index].time = 0;
		}
		for (int i = 0; i < nextAnimations[index].index.size(); ++i) {
			if (nextAnimations[index].index[i] == idx) {
				nextAnimations[index].weight[i] += 1;
				return;
			}
		}
		nextAnimations[index].index.push_back(GetAnimationIndex(nextAnim));
		nextAnimations[index].weight.push_back(weight);
	}
}

void KG::Component::AnimationControllerComponent::SetAnimationWeight(int index, const KG::Utill::HashString anim, int weight)
{
	if (index >= nextAnimations.size())
		return;
	else if (index == -1) {
		int idx = GetAnimationCommandIndex(anim, index);
		if (idx == -1)
			return;
		else {
			if (curAnimation.weight.size() <= idx) {
				curAnimation.weight.resize(idx + 1);
			}
			curAnimation.weight[idx] = weight;
		}
	}
	else {
		int idx = GetAnimationCommandIndex(anim, index);
		if (idx == -1)
			return;
		else {
			if (nextAnimations[index].weight.size() <= idx) {
				nextAnimations[index].weight.resize(idx + 1);
			}
			nextAnimations[index].weight[idx] = weight;
		}
	}
}