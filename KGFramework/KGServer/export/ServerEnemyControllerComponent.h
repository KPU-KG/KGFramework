#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include "SerializableProperty.h"
#include <functional>

namespace KG::Component
{
	// 1. �־��� ��ġ���� ���� ���� - �̰� GUI �ʿ��ұ�?
	// 2. ���� ������ ���ƴٴϱ� - �̰� ��� ��ġ�ϸ� A*�� ��ã�� �˰��� �����ؾ� �ҵ�

	class DLL SEnemyControllerComponent : public SBaseComponent
	{
	private:
		DirectX::XMFLOAT3		center = { 0,0,0 };			// onCreate���� ������
		float					range = 10;
		DirectX::XMFLOAT3		direction = { 0,0,0 };		// �ϴ� z���� ������� ���� �����̳� ���� ������ ������..?
		DirectX::XMFLOAT3		goal = { 0,0,0 };
		float					speed = 10;
		float					watingTime = 3;
	public:
		SEnemyControllerComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
	private:
		// property
		KG::Core::SerializableProperty<float>					rangeProp;
		KG::Core::SerializableProperty<float>					speedProp;

	public:
		virtual bool OnDrawGUI();
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;

	};

	REGISTER_COMPONENT_ID(SEnemyControllerComponent);

	class DLL SEnemyControllerComponentSystem : public KG::Component::SBaseComponentSystem<SEnemyControllerComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for (auto& com : *this)
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem��(��) ���� ��ӵ�
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}