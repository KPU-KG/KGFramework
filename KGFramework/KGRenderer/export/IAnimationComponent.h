#pragma once
#include <vector>
#include <unordered_map>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"

#define ANIMSTATE_PLAYING 0
#define ANIMSTATE_CHANGING 1
#define ANIMSTATE_FORCE 2

#define ANIMLOOP_INF -1

#define ANIMINDEX_CURRENT -1
#define ANIMINDEX_CHANGE 0

namespace KG::Core
{
	class GameObject;
};

namespace KG::Utill
{
	struct AnimationSet;
};

namespace KG::Component
{
	class Render3DComponent;
	class GeometryComponent;

	class DLL IBoneTransformComponent : public IRenderComponent
	{
	public:
		virtual KG::Core::GameObject* BoneIndexToGameObject( UINT index, UINT submeshIndex = 0 ) const = 0;
		virtual void SetRootNode(KG::Core::GameObject* object) = 0;
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
	};

	struct Animation {
	private:
		void MatchNode(KG::Core::GameObject* gameObject, UINT animIndex = 0U);
		void SetDuration(KG::Utill::AnimationSet* anim);
	public:
		// int animIndex = 0;
		// bool isRegistered = false;
		KG::Utill::HashString animationId;
		std::vector<std::vector<KG::Core::GameObject*>> frameCache;
		float timer = 0.0f;
		float duration = 0.0f;
		void Initialize(KG::Core::GameObject* gameObject, UINT animIndex = 0U);
	};

	// struct AnimCommandValue {
	// 	UINT animIndex;
	// 	int weight;
	// };

	struct AnimationCommand {
		using AnimCommandValue = std::unordered_map<UINT, int>;
		std::unordered_map<KG::Utill::hashType, AnimCommandValue> index;
		float duration = 0.1f;
		int repeat = -1;
		float time = 0.0f;
		float speed = 1.0f;
		int next = ANIMSTATE_PLAYING;
		bool applyTransform = true;
		bool applyRotation = true;
		bool applyScale = true;
	};

	struct AnimationEvent {
	private:
	public:
		AnimationEvent(const KG::Utill::HashString& eventId, float time) : eventId(eventId), time(time) { }
		// KG::Utill::HashString animationId;
		// int keyFrame;
		float time; // 이벤트 등록할 때 키 프레임으로 받아서 타이밍을 계산한 뒤 저장
		KG::Utill::HashString eventId;		// 현재는 사운드만 생각하고 해쉬 스트링 형태로 저장
		bool activated = false;
	};

	class DLL IAnimationControllerComponent : public IRenderComponent
	{
	public:
		virtual void Update(float timeElapsed) override = 0;
		virtual void RegisterAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U) = 0;
		virtual void RegisterEvent(const KG::Utill::HashString& animationId, int keyFrame, const KG::Utill::HashString& eventId) = 0;

		virtual void SetDefaultAnimation(KG::Utill::HashString defaultAnim, UINT animationIndex = 0U) = 0;
		virtual void SetAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int repeat = -1, float speed = 1.0f, bool clearNext = true, int weight = 1) = 0;
		virtual int ChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f) = 0;
		virtual int AddNextAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, int repeat = 0.1f, float speed = 1.0f, int weight = 1) = 0;
		virtual void BlendingAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex = 0U, float duration = -1.f, int index = -1, int weight = 1) = 0;
		virtual int ForceChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f) = 0;

		virtual void SetAnimationWeight(int index, const KG::Utill::HashString& anim, UINT animationIndex = 0U, int weight = 1) = 0;
		virtual void SetIgnoreScale(bool isUsing) = 0;
		virtual void SetIgnoreTranslate(bool isUsing) = 0;
		virtual bool OnDrawGUI() override = 0;

		virtual float GetDuration(const KG::Utill::HashString& animId, UINT animationIndex = 0U) = 0;
		virtual KG::Utill::HashString GetCurrentPlayingAnimationId() const = 0;
		virtual UINT GetCurrentPlayingAnimationIndex() const = 0;
		virtual float GetCurrentPlayingAnimationTime() const = 0;
		virtual float GetCurrentPlayingAnimationDuration() const = 0;
	};

	REGISTER_COMPONENT_ID(IBoneTransformComponent );
	REGISTER_COMPONENT_ID(IAnimationControllerComponent);
};
