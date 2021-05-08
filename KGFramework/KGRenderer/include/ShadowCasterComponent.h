#pragma once
#include <DirectXMath.h>
#include "IRenderComponent.h"
#include "IShadowCasterComponent.h"
#include "IDXRenderComponent.h"

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

	class DLL ShadowCasterComponent : public IShadowCasterComponent, IDXRenderComponent
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
		virtual bool isPointLightShadow() const override { return this->pointLightCamera != nullptr; };
		virtual bool isDirectionalLightShadow() const override { return this->directionalLightCamera != nullptr; };
		virtual bool isSpotLightShadow() const override { return this->spotLightCamera != nullptr; };

		KG::Renderer::RenderTexture& GetRenderTexture();
		KG::Component::GSCubeCameraComponent* GetPointLightCamera() const;
		KG::Component::GSCascadeCameraComponent* GetDirectionalLightCamera() const;
		KG::Component::CameraComponent* GetSpotLightCamera() const;
		KG::Component::LightType GetTargetLightType() const;

		//Serialize Part
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
	};
    REGISTER_COMPONENT_ID_REPLACE( ShadowCasterComponent, IShadowCasterComponent);
};
