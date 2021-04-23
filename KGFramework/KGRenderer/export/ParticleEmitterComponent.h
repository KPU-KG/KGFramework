#pragma once
#include <vector>
#include "IRenderComponent.h"

namespace KG::Renderer
{
	struct KGRenderJob;
};

namespace KG::Component
{

	// ��ƼŬ ����ũ ä���� Emit
	// �������� ���� ���ǵ� ��ƼŬ ���� (5���� ����) �� ���ۿ� �Ҵ� ������
	// �������� Update��� ������ ��ƼŬ�� ��ȸ ���鼭 ������ Ÿ�� ���� ����� ����

	// �׸��� ���� �ÿ��� ����ũ ������Ʈ���� �� ���� ũ�� ��ŭ �׸��� ȣ�� ��
	// �ִϸ��̼� Ʈ������ ���Կ� ��ƼŬ �̹��� ���� �ȱ�
	// ��ƼŬ �̹��� ���� ��ǥ�� ������Ʈ�� ���̴����� ������ ����
	// Emission�� ���� ������� �ʴ´�.
	// �ϴ� ��ƼŬ�� ���̴� �Ѱ� ����ϰ� �սô�
	// ���� ��� X
	


	struct ParticleDesc
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 speed;
		DirectX::XMFLOAT4 acc;
		DirectX::XMFLOAT4 color;
		double startTime;
		double lifeTime;
	};

	class DLL ParticleEmitterComponent : public IRenderComponent
	{
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		virtual void OnRender(ID3D12GraphicsCommandList* commadList) override;
		virtual void OnPreRender() override;
	public:
		void EmitParticle(const ParticleDesc& desc);
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID(ParticleEmitterComponent);
};
