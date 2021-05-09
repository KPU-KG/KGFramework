#pragma once
#include <vector>
#include <unordered_map>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IAnimationComponent.h"
#include "IDXRenderComponent.h"

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

	class BoneTransformComponent : public IBoneTransformComponent, IDXRenderComponent
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

	class AnimationControllerComponent : public IAnimationControllerComponent, IDXRenderComponent
	{
		using AnimationEventSet = std::vector<AnimationEvent>;
		// changing
		// playing
		// stop
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
		virtual void RegisterAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U) override;
		virtual void RegisterEvent(const KG::Utill::HashString& animationId, int keyFrame, const KG::Utill::HashString& eventId) override;

		virtual void SetDefaultAnimation(KG::Utill::HashString defaultAnim, UINT animationIndex = 0U) override;
		virtual void SetAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int repeat = -1, float speed = 1.0f, bool clearNext = true, int weight = 1) override;
		virtual int ChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f) override;
		virtual int AddNextAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, int repeat = 0.1f, float speed = 1.0f, int weight = 1) override;
		virtual void BlendingAnimation(const KG::Utill::HashString& nextAnim, UINT animationIndex = 0U, float duration = -1.f, int index = -1, int weight = 1) override;
		virtual int ForceChangeAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, int repeat = 1, bool addWeight = false, float speed = 1.0f) override;

		virtual void SetAnimationWeight(int index, const KG::Utill::HashString& anim, UINT animationIndex = 0U, int weight = 1) override;
		virtual void SetIgnoreScale(bool isUsing) override;
		virtual void SetIgnoreTranslate(bool isUsing) override;
		virtual bool OnDrawGUI() override;

		virtual float GetDuration(const KG::Utill::HashString& animId, UINT animationIndex = 0U) override;
		virtual KG::Utill::HashString GetCurrentPlayingAnimationId() const override;
		virtual UINT GetCurrentPlayingAnimationIndex() const override;
		virtual float GetCurrentPlayingAnimationTime() const override;
		virtual float GetCurrentPlayingAnimationDuration() const override;

        // IAnimationControllerComponent��(��) ���� ��ӵ�
        //virtual void SyncAnimation(const KG::Utill::HashString& anim, UINT animationIndex, float timer) override;
    };

    REGISTER_COMPONENT_ID_REPLACE(BoneTransformComponent, IBoneTransformComponent);
    REGISTER_COMPONENT_ID_REPLACE(AnimationControllerComponent, IAnimationControllerComponent);
};
