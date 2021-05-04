#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include <DirectXCollision.h>

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
	

	class TransformComponent;
	struct ParticleDesc
	{
		DirectX::XMFLOAT3 position;
		float rotation;
		DirectX::XMFLOAT3 speed;
		float rotationSpeed;
		DirectX::XMFLOAT2 size;
		UINT materialIndex;
		float pad1;
		DirectX::XMFLOAT4 color;
		double startTime;
		float lifeTime;
		float pad0;
	};

	class DLL ParticleEmitterComponent : public IRenderComponent
	{
		float emitTimer = 0.0f;
		TransformComponent* transform = nullptr;
		KG::Utill::HashString particleMaterial;
		UINT particleMaterialIndex = 0;
		bool isParticleAdd = false;


		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		ParticleEmitterComponent();
		virtual void OnRender(ID3D12GraphicsCommandList* commadList) override;
		virtual void OnPreRender() override;
	public:
		DirectX::XMFLOAT3 baseDeltaPosition = DirectX::XMFLOAT3(0,0,0);
		DirectX::XMFLOAT3 rangeDeltaPosition = DirectX::XMFLOAT3(0, 0, 0);

		DirectX::XMFLOAT2 baseSize = DirectX::XMFLOAT2(1, 1);
		DirectX::XMFLOAT2 rangeSize = DirectX::XMFLOAT2(0, 0);

		DirectX::XMFLOAT3 baseSpeed = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT3 rangeSpeed = DirectX::XMFLOAT3(0, 0, 0);

		DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1,1,1,1);

		float baselifeTime = 10;
		float rangelifeTime = 0;

		float baseRotation = 0;
		float rangeRotation = 0;

		float baseRotationSpeed = 0;
		float rangeRotationSpeed = 0;

		float emitPerSecond = 0;

		float baseEmitCount = 0;
		float rangeEmitCount = 0;


		void EmitParticle();
		void EmitParticle(const ParticleDesc& desc, bool autoFillTime);
		UINT GetParticleMaterialIndex(const KG::Utill::HashString& id) const;
		bool GetParticleMaterialIsAdd(const KG::Utill::HashString& id) const;
	private:
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> baseDeltaPositionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> rangeDeltaPositionProp;

		KG::Core::SerializableProperty<DirectX::XMFLOAT2> baseSizeProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT2> rangeSizeProp;


		KG::Core::SerializableProperty<DirectX::XMFLOAT3> baseSpeedProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> rangeSpeedProp;

		KG::Core::SerializableProperty<DirectX::XMFLOAT4> colorProp;

		KG::Core::SerializableProperty<float> baselifeTimeProp;
		KG::Core::SerializableProperty<float> rangelifeTimeProp;

		KG::Core::SerializableProperty<float> baseRotationProp;
		KG::Core::SerializableProperty<float> rangeRotationProp;

		KG::Core::SerializableProperty<float> baseRotationSpeedProp;
		KG::Core::SerializableProperty<float> ramgeRotationSpeedProp;

		KG::Core::SerializableProperty<float> emitPerSecondProp;

		KG::Core::SerializableProperty<float> baseEmitCountSecondProp;
		KG::Core::SerializableProperty<float> rangeEmitCountSecondProp;

		KG::Core::SerializableProperty<KG::Utill::HashString> particleMaterialProp;

	public:
		virtual void Update(float elapsedTime) override;
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
		void SetParticleMaterial(const KG::Utill::HashString& materialId);
	};
	REGISTER_COMPONENT_ID(ParticleEmitterComponent);
};
