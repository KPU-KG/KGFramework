#pragma once
#include <DirectXMath.h>
#include "IRenderComponent.h"
#include "LightComponent.h"

namespace KG::Renderer
{
	struct RenderTexture;
	struct RenderTextureDesc;
};

namespace KG::Component
{
	class CameraComponent;
	class LightComponent;
	class GSCubeCameraComponent;
	class GSCascadeCameraComponent;

	class DLL ShadowCasterComponent : public IRenderComponent
	{
		KG::Component::LightComponent* targetLight = nullptr;
		KG::Component::GSCubeCameraComponent* pointLightCamera = nullptr;
		KG::Component::GSCascadeCameraComponent* directionalLightCamera = nullptr;
		KG::Component::CameraComponent* spotLightCamera = nullptr;

		KG::Component::CameraComponent* mainCamera = nullptr;

		void InitializeAsPointLightShadow();
		void InitializeAsDirectionalLightShadow();
		void InitializeAsSpotLightShadow();
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;
	public:
		virtual void OnPreRender() override;
		bool isPointLightShadow() const { return this->pointLightCamera != nullptr; };
		bool isDirectionalLightShadow() const { return this->directionalLightCamera != nullptr; };
		bool isSpotLightShadow() const { return this->spotLightCamera != nullptr; };

		void SetTargetCameraCamera( KG::Component::CameraComponent* mainCamera );

		KG::Renderer::RenderTexture& GetRenderTexture();
		KG::Component::GSCubeCameraComponent* GetPointLightCamera() const;
		KG::Component::GSCascadeCameraComponent* GetDirectionalLightCamera() const;
		KG::Component::CameraComponent* GetSpotLightCamera() const;
		KG::Component::LightType GetTargetLightType() const;
	};

	REGISTER_COMPONENT_ID( ShadowCasterComponent );

};
