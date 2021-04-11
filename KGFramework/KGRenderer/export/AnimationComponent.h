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

	class DLL BoneTransformComponent : public IRenderComponent
	{
		friend Render3DComponent;
		using FrameCacheVector = std::vector<KG::Core::GameObject*>;
	protected:
		KG::Component::GeometryComponent* geometry = nullptr;
		std::vector<FrameCacheVector> frameCache;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		KG::Core::GameObject* rootNode = nullptr;
		KG::Utill::HashString rootNodeTag;
		KG::Core::SerializableProperty<KG::Utill::HashString> rootNodeIdProp;
		void InitializeBone( KG::Core::GameObject* rootNode );
	public:
		BoneTransformComponent();
		KG::Core::GameObject* BoneIndexToGameObject( UINT index, UINT submeshIndex = 0 ) const;
		void SetRootNode(KG::Core::GameObject* object);
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};

	struct Animation {
	private:
		void MatchNode(KG::Core::GameObject* gameObject);
		void SetDuration(KG::Utill::AnimationSet* anim);
	public:
		bool isRegistered = false;
		KG::Utill::HashString animationId;
		std::vector<std::vector<KG::Core::GameObject*>> frameCache;
		float timer = 0.0f;
		float duration = 0.0f;
		void Initialize(KG::Core::GameObject* gameObject);
	};

	struct AnimationCommand {
		// std::vector<int> index;
		// std::vector<int> weight;
		std::unordered_map<KG::Utill::hashType, int> index;
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

	class DLL AnimationControllerComponent : public IRenderComponent
	{
		using AnimationEventSet = std::vector<AnimationEvent>;
		// changing
		// playing
	protected:
		int state = ANIMSTATE_PLAYING;
		bool changeIntercepted = false;
		bool changeToDefault = false;
		bool isIgnoreScale = true;
		bool isIgnoreTranslate = false;
		std::vector<DirectX::XMFLOAT4> prevFrameCache;

		// std::vector<Animation> animations;
		std::unordered_map<KG::Utill::hashType, Animation> animations;
		std::unordered_map<KG::Utill::hashType, AnimationEventSet> events;

		Animation* curFrame = nullptr;
		AnimationCommand curAnimation;
		std::vector<AnimationCommand> nextAnimations;

		KG::Utill::HashString defaultAnimation;

		bool IsValidAnimationId(const KG::Utill::HashString& animationId);
		int GetTotalWeight(int index);
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
		virtual void OnDestroy() override;
		void PlayingUpdate(float elapsedTime);
		void ChangingUpdate(float elapsedTime);
	public:
		virtual void Update(float timeElapsed) override;
		void RegisterAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U);
		void RegisterEvent(const KG::Utill::HashString& animationId, int keyFrame, const KG::Utill::HashString& eventId);

		void SetDefaultAnimation(KG::Utill::HashString defaultAnim);
		void SetAnimation(const KG::Utill::HashString& animationId, int repeat = -1, float speed = 1.0f, bool clearNext = true, int weight = 1);
		int ChangeAnimation(const KG::Utill::HashString& animationId, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f);
		int AddNextAnimation(const KG::Utill::HashString& nextAnim, int nextState = ANIMSTATE_PLAYING, int repeat = 0.1f, float speed = 1.0f, int weight = 1);
		void BlendingAnimation(const KG::Utill::HashString& nextAnim, float duration = -1.f, int index = -1, int weight = 1);
		int ForceChangeAnimation(const KG::Utill::HashString& animationId, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f);

		void SetAnimationWeight(int index, const KG::Utill::HashString& anim, int weight);
		void SetIgnoreScale(bool isUsing);
		void SetIgnoreTranslate(bool isUsing);
		virtual bool OnDrawGUI() override;

		float GetDuration(const KG::Utill::HashString& animId);
		KG::Utill::HashString GetCurrentPlayingAnimationId() const;
		float GetCurrentPlayingAnimationTime() const;
		float GetCurrentPlayingAnimationDuration() const;
	};

	REGISTER_COMPONENT_ID( BoneTransformComponent );
	REGISTER_COMPONENT_ID(AnimationControllerComponent);
};
