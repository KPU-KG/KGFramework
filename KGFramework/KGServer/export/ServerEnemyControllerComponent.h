#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include "SerializableProperty.h"
#include <functional>

namespace KG::Component
{
	// 1. 주어진 위치에서 범위 설정 - 이거 GUI 필요할까?
	// 2. 범위 내에서 돌아다니기 - 이건 노드 설치하면 A*나 길찾기 알고리즘 적용해야 할듯

	class DLL SEnemyControllerComponent : public SBaseComponent
	{
	private:
		DirectX::XMFLOAT3		center = { 0,0,0 };			// onCreate에서 정해줌
		float					range = 10;
		DirectX::XMFLOAT3		direction = { 0,0,0 };		// 일단 z값은 고려하지 않을 예정이나 비행 몹에는 쓸지도..?
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

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}