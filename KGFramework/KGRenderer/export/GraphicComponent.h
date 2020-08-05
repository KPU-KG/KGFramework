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
	class IShader;
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

	class DLL IRenderComponent : public IComponent
	{
	public:
		virtual void OnPreRender() {};
		virtual void OnRender() {};
	};

	class DLL CameraComponent : public IRenderComponent
	{

	};

	class DLL GeometryComponent : public IRenderComponent
	{
	};

	class DLL Render3DComponent : public IRenderComponent
	{
		TransformComponent* transform = nullptr;
		GeometryComponent* geometry = nullptr;
		MaterialComponent* material = nullptr;
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
		void RegisterMaterial(MaterialComponent* material);
 	};

	class DLL LightComponent : public IRenderComponent
	{

	};

	class DLL MaterialComponent : public IRenderComponent
	{
	protected:
		KG::System::ISystem* materialSystem = nullptr;
		std::vector<KG::Renderer::IShader*> shaders;
		std::vector<KG::Renderer::MaterialConstant*> shaderDatas;
		virtual void OnDestroy() override;
	public:
		void InitializeShader(const KG::Utill::HashString& shaderID);
	};
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