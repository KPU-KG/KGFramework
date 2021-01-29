#pragma once
#include <vector>
#include "IRenderComponent.h"

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

	class DLL AnimationStreamerComponent : public IRenderComponent
	{
		using FrameCacheVector = std::vector<KG::Core::GameObject*>;
	protected:
		KG::Utill::AnimationSet* anim = nullptr;
		KG::Component::GeometryComponent* geometry = nullptr;
		std::vector<FrameCacheVector> frameCache;
		float timer = 0.0f;
		float duration = 0.0f;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		void MatchNode();
	public:
		virtual void Update( float timeElapsed ) override;
		void InitializeAnimation( const KG::Utill::HashString& animationId, UINT animationIndex = 0 );
		void GetDuration();
	};

	REGISTER_COMPONENT_ID( BoneTransformComponent );
	REGISTER_COMPONENT_ID( AnimationStreamerComponent );
};
