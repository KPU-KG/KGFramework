#pragma once
#include <vector>
#include <DirectXCollision.h>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IParticleEmitterComponent.h"
#include "IDXRenderComponent.h"

namespace KG::Renderer
{
    class ParticleGenerator;
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
	

	class TransformComponent;
   
	class ParticleEmitterComponent : public IParticleEmitterComponent, IDXRenderComponent
	{
		float emitTimer = 0.0f;
		TransformComponent* transform = nullptr;
        KG::Renderer::ParticleGenerator* particleGenerator = nullptr;

		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		ParticleEmitterComponent();
		virtual void OnRender(ID3D12GraphicsCommandList* commadList) override;
		virtual void OnPreRender() override;
	public:
        virtual void AddParticleDesc(const KG::Utill::HashString& id, const ParticleDesc& desc) override;
        virtual void EmitParticle(const KG::Utill::HashString& id) override;
		virtual void EmitParticle(const ParticleData& desc, bool autoFillTime, ParticleType type) override;
		UINT GetParticleMaterialIndex(const KG::Utill::HashString& id) const;
		bool GetParticleMaterialIsAdd(const KG::Utill::HashString& id) const;
	private:
        ParticleDesc previewParticle;
        /* preview */
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> baseDeltaPositionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> rangeDeltaPositionProp;

		KG::Core::SerializableProperty<DirectX::XMFLOAT2> baseSizeProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT2> rangeSizeProp;


		KG::Core::SerializableProperty<DirectX::XMFLOAT3> baseSpeedProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> rangeSpeedProp;

		KG::Core::SerializableProperty<KG::Utill::Color> colorProp;

		KG::Core::SerializableProperty<float> baselifeTimeProp;
		KG::Core::SerializableProperty<float> rangelifeTimeProp;

		KG::Core::SerializableProperty<float> baseRotationProp;
		KG::Core::SerializableProperty<float> rangeRotationProp;

		KG::Core::SerializableProperty<float> baseRotationSpeedProp;
		KG::Core::SerializableProperty<float> ramgeRotationSpeedProp;

		KG::Core::SerializableProperty<float> baseEmitCountSecondProp;
        KG::Core::SerializableProperty<float> rangeEmitCountSecondProp;
        KG::Core::SerializableProperty<KG::Utill::HashString> materialIdProp;

		//KG::Core::SerializableProperty<float> emitPerSecondProp; // ���߿� �ֱ��� ���� �� �� ��
	public:
		virtual void Update(float elapsedTime) override;
    public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();

        virtual void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position) override;
        virtual void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 direction) override;

    };
	REGISTER_COMPONENT_ID_REPLACE(ParticleEmitterComponent, IParticleEmitterComponent);
};
