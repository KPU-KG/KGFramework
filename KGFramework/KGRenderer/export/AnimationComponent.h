#pragma once
#include <vector>
#include "IRenderComponent.h"

#define ANIMSTATE_PLAYING 0
#define ANIMSTATE_CHANGING 1

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
	public:
		KG::Core::GameObject* BoneIndexToGameObject( UINT index, UINT submeshIndex = 0 ) const;
		void InitializeBone( KG::Core::GameObject* rootNode );
	};

	struct Animation {
	private:
		void MatchNode(KG::Core::GameObject* gameObject);
		void SetDuration(KG::Utill::AnimationSet* anim);
	public:
		KG::Utill::HashString animationId;
		std::vector<std::vector<KG::Core::GameObject*>> frameCache;
		float timer = 0.0f;
		float duration = 0.0f;
		void Initialize(KG::Core::GameObject* gameObject);
	};

	struct AnimationCommand {
		std::vector<int> index;
		std::vector<int> weight;
		float duration = 0.1f;
		float time = 0.0f;
		float speed = 0.5f;
		int next = ANIMSTATE_PLAYING;
		bool applyTransform = false;
		bool applyRotation = true;
		bool applyScale = true;
	};

	class DLL AnimationControllerComponent : public IRenderComponent
	{
		// changing
		// playing
	protected:
		int state = ANIMSTATE_PLAYING;
		std::vector<Animation> animations;

		AnimationCommand curAnimation;
		std::vector<AnimationCommand> nextAnimations;

		KG::Utill::HashString defaultAnimation;

		int GetAnimationIndex(const KG::Utill::HashString& animationId);
		int GetAnimationCommandIndex(const KG::Utill::HashString animationId, int index);
		int GetTotalWeight(int index);
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
		virtual void OnDestroy() override;
		void PlayingUpdate(float elapsedTime);
		void ChangingUpdate(float elapsedTime);
	public:
		virtual void Update(float timeElapsed) override;
		void RegisterAnimation(const KG::Utill::HashString& animationId, UINT animationIndex = 0U);
		void SetDefaultAnimation(KG::Utill::HashString defaultAnim);
		void SetAnimation(const KG::Utill::HashString& animationId, float duration = -1, float speed = 0.5f, bool clearNext = true, int weight = 1);
		int ChangeAnimation(const KG::Utill::HashString& animationId, int nextState = ANIMSTATE_PLAYING, float blendingDuration = 0.1f, float animationDuration = 0.5f, float speed = 0.5f);
		int AddNextAnimation(const KG::Utill::HashString nextAnim, int nextState = ANIMSTATE_PLAYING, float duration = 0.1f, float speed = 0.5f, int weight = 1);
		void BlendingAnimation(const KG::Utill::HashString nextAnim, float duration = -1.f, int index = -1, int weight = 1);
		void SetAnimationWeight(int index, const KG::Utill::HashString anim, int weight);
	};

	REGISTER_COMPONENT_ID( BoneTransformComponent );
	REGISTER_COMPONENT_ID(AnimationControllerComponent);
};
