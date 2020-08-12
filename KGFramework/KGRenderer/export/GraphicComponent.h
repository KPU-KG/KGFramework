#pragma once
#include "IComponent.h"
#include <vector>

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
	class Shader;
	class Geometry;
	struct MaterialConstant;
}
namespace KG::System
{
	class ISystem;
}

namespace KG::Component
{
	class TransformComponent;
	class MaterialComponent;
	class GeometryComponent;
	class Render3DComponent;

	class DLL IRenderComponent : public IComponent
	{
	public:
		virtual void OnPreRender() {};
		virtual void OnRender() {};
	};

	class DLL CameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
	};


	class DLL Render3DComponent : public IRenderComponent
	{
		TransformComponent* transform = nullptr;
		GeometryComponent* geometry = nullptr;
		MaterialComponent* material = nullptr;
		KG::Renderer::KGRenderJob* renderJob = nullptr;
	public:
		bool isVisible = true;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnRender() override;
		virtual void OnPreRender() override;
		void SetVisible( bool visible );
		void SetRenderJob( KG::Renderer::KGRenderJob* renderJob );
		void RegisterTransform( TransformComponent* transform );
		void RegisterMaterial( MaterialComponent* material );
		void RegisterGeometry( GeometryComponent* geometry );
	};

	class DLL GeometryComponent : public IRenderComponent
	{
		friend Render3DComponent;
		KG::Renderer::Geometry* geometry = nullptr;
	public:
		void InitializeGeometry( const KG::Utill::HashString& shaderID );
	};
	class DLL LightComponent : public IRenderComponent
	{
		friend Render3DComponent;

	};

	class DLL MaterialComponent : public IRenderComponent
	{
		friend Render3DComponent;
	protected:
		KG::System::ISystem* materialSystem = nullptr;
		KG::Renderer::Shader* shaders = nullptr;
		KG::Renderer::MaterialConstant* shaderDatas = nullptr;
		virtual void OnDestroy() override;
	public:
		void InitializeShader( const KG::Utill::HashString& shaderID );
		unsigned GetMaterialIndex() const;
	};

	REGISTER_COMPONENT_ID( KG::Component::IRenderComponent );
	REGISTER_COMPONENT_ID( KG::Component::CameraComponent );
	REGISTER_COMPONENT_ID( KG::Component::Render3DComponent );
	REGISTER_COMPONENT_ID( KG::Component::LightComponent );
	REGISTER_COMPONENT_ID( KG::Component::MaterialComponent );
	REGISTER_COMPONENT_ID( KG::Component::GeometryComponent );
}
//���� �ؽ�ó ���� ���� ��ũ���� ���� ��ġ
//���׸��� ��� ���̴� �ö󰥶� ����  CBV 2��
//������Ʈ ��� 1���� SRV�� ���� SRV 0��
//ī�޶�� ��� CBV 0��
//���̴� �н� ������ CBV 1��

//�ؽ�ó���� ū ��ũ���� ���� �ΰ��� �з��� �ɰ��� ���� (ť����� �ʿ��ϸ�?? ) 
//���� �����̽� 1�� SRV0 /  �����̽�2 �� SRV0 ���� �����ϴ� �迭
// ���׸��󿡼��� �ε��� ������ �� �����̽����� �ؽ�ó ���ܿ�
// 