#pragma once
#include <vector>
#include <unordered_map>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IAnimationComponent.h"

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
	class FakeGeometryComponent;

	class FakeBoneTransformComponent : public IBoneTransformComponent
	{
		friend Render3DComponent;
		using FrameCacheVector = std::vector<KG::Core::GameObject*>;
	protected:
		KG::Component::FakeGeometryComponent* geometry = nullptr;
		std::vector<FrameCacheVector> frameCache;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		KG::Core::GameObject* rootNode = nullptr;
		KG::Utill::HashString rootNodeTag;
		KG::Core::SerializableProperty<KG::Utill::HashString> rootNodeIdProp;
		void InitializeBone( KG::Core::GameObject* rootNode );
	public:
        FakeBoneTransformComponent();
		virtual KG::Core::GameObject* BoneIndexToGameObject( UINT index, UINT submeshIndex = 0 ) const;
		virtual void SetRootNode(KG::Core::GameObject* object) override;
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
		virtual bool OnDrawGUI() override;
	};

	class FakeAnimationControllerComponent : public IAnimationControllerComponent
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
		using Animations = std::unordered_map<UINT, Animation>;
		std::unordered_map<KG::Utill::hashType, Animations> animations;
		std::unordered_map<KG::Utill::hashType, AnimationEventSet> events;

		Animation* curFrame = nullptr;
		AnimationCommand curAnimation;
		std::vector<AnimationCommand> nextAnimations;

		std::pair<KG::Utill::HashString, UINT> defaultAnimation;

		bool IsValidAnimationId(const KG::Utill::HashString& animationId, UINT animIndex = 0U);
		int GetTotalWeight(int index);
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
		virtual void OnDestroy() override;
		void PlayingUpdate(float elapsedTime);
		void ChangingUpdate(float elapsedTime);
	public:
		virtual void Update(float timeElapsed) override;
		virtual void RegisterAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U);
		virtual void RegisterEvent(const KG::Utill::HashString& animationId, int keyFrame, const KG::Utill::HashString& eventId);

		virtual void SetDefaultAnimation(KG::Utill::HashString defaultAnim, UINT animationIndex = 0U);
		virtual void SetAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int repeat = -1, float speed = 1.0f, bool clearNext = true, int weight = 1);
		virtual int ChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f);
		virtual int AddNextAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, int repeat = 0.1f, float speed = 1.0f, int weight = 1);
		virtual void BlendingAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex = 0U, float duration = -1.f, int index = -1, int weight = 1);
		virtual int ForceChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f);

		virtual void SetAnimationWeight(int index, const KG::Utill::HashString& anim, UINT animationIndex = 0U, int weight = 1);
		virtual void SetIgnoreScale(bool isUsing);
		virtual void SetIgnoreTranslate(bool isUsing);
		virtual bool OnDrawGUI() override;

		virtual float GetDuration(const KG::Utill::HashString& animId, UINT animationIndex = 0U);
		virtual KG::Utill::HashString GetCurrentPlayingAnimationId() const;
		virtual UINT GetCurrentPlayingAnimationIndex() const;
		virtual float GetCurrentPlayingAnimationTime() const;
		virtual float GetCurrentPlayingAnimationDuration() const;

        // IAnimationControllerComponent을(를) 통해 상속됨
        //virtual void SyncAnimation(const KG::Utill::HashString& anim, UINT animationIndex, float timer) override;
    };

    REGISTER_COMPONENT_ID_REPLACE(FakeBoneTransformComponent, IBoneTransformComponent);
    REGISTER_COMPONENT_ID_REPLACE(FakeAnimationControllerComponent, IAnimationControllerComponent);
};
