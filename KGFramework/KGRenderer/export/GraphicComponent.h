#pragma once
#include "IComponent.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // GRAPHICPART_EXPORTS

namespace KG::Core
{
	class GameObject;
};
namespace KG::Renderer
{
	class KGRenderJob;
}

namespace KG::Component
{
	class TransformComponent;

	class DLL IRenderComponent : public IComponent
	{
	public:
		virtual void OnPreRender() {};
		virtual void OnRender() {};
	};

	class DLL CameraComponent : public IRenderComponent
	{

	};

	class DLL Render3DComponent : public IRenderComponent
	{
		TransformComponent* transform = nullptr;
		KG::Renderer::KGRenderJob* renderJob = nullptr;
	protected:
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		bool isVisible = true;
		virtual void OnRender() override;
		virtual void OnPreRender() override;
		void SetVisible(bool visible);
		void SetRenderJob(KG::Renderer::KGRenderJob* renderJob);
		void RegisterTransform(TransformComponent* transform);
 	};

	class DLL LightComponent : public IRenderComponent
	{

	};

}