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
	class LightComponent;
	class CubeCameraComponent;
	class CameraComponent;
	class GSCubeCameraComponent;

	class ShadowCasterComponent : public IRenderComponent
	{
		friend LightComponent;
		GSCubeCameraComponent* cubeCamera = nullptr;
		CameraComponent* camera = nullptr;
		void InitializeAsPointLightShadow(KG::Component::LightComponent* light);
		void InitializeAsDirectionalLightShadow( KG::Component::LightComponent* light );
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;
	public:
		bool isPointLightShadow() const { return this->cubeCamera != nullptr; };
		bool isDirectionalLightShadow() const { return this->camera != nullptr; };
		KG::Renderer::RenderTexture& GetRenderTexture();
		GSCubeCameraComponent* GetCubeCamera() const;
		CameraComponent* GetCamera() const;
	};
	REGISTER_COMPONENT_ID( ShadowCasterComponent );

};
