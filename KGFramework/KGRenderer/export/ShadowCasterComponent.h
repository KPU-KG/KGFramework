#pragma once
#include <DirectXMath.h>
#include "IRenderComponent.h"

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
		KG::Component::GSCubeCameraComponent* cubeCamera = nullptr;
		KG::Component::GSCascadeCameraComponent* camera = nullptr;
		KG::Component::CameraComponent* mainCamera = nullptr;

		void InitializeAsPointLightShadow();
		void InitializeAsDirectionalLightShadow();
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;
	public:
		virtual void OnPreRender() override;
		bool isPointLightShadow() const { return this->cubeCamera != nullptr; };
		bool isDirectionalLightShadow() const { return this->camera != nullptr; };

		void SetTargetCameraCamera( KG::Component::CameraComponent* mainCamera );

		KG::Renderer::RenderTexture& GetRenderTexture();
		KG::Component::GSCubeCameraComponent* GetCubeCamera() const;
		KG::Component::GSCascadeCameraComponent* GetCamera() const;
	};

	REGISTER_COMPONENT_ID( ShadowCasterComponent );

};
