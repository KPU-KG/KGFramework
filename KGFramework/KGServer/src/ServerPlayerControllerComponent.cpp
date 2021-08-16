#include "pch.h"
#include "ServerPlayerControllerComponent.h"
#include "ServerEnemyCrawlerComponent.h"
#include "Transform.h"
#include "KGServer.h"
#include "PhysicsComponent.h"
#include "PhysicsScene.h"

using namespace KG::Math::Literal;

static enum KeyState
{
	None,
	Down,
	Pressing,
	Up,
};

static bool IsTouching(unsigned char state)
{
	return state == KeyState::Down || state == KeyState::Pressing;
}


void KG::Component::SPlayerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->trasnform = this->GetGameObject()->GetComponent<TransformComponent>();
	this->rotationTrasnform = this->GetGameObject()->GetChild()->GetTransform();
	this->physics = this->gameObject->GetComponent<DynamicRigidComponent>();
	this->physics->SetCollisionCallback([this](KG::Component::IRigidComponent* my, KG::Component::IRigidComponent* other) {
		DebugNormalMessage("Player object collide");
		auto filterMy = my->GetFilterMask();
		auto filterOther = other->GetFilterGroup();
		if (filterOther & static_cast<uint32_t>(FilterGroup::eBULLET)) {
			this->HitBullet(1);
		}
		if (filterOther & static_cast<uint32_t>(FilterGroup::eENEMY)) {
			auto comp = other->GetGameObject()->GetComponent<SEnemyCrawlerComponent>();
			if (comp != nullptr) {
				if (comp->IsCharging() && !this->hitFlag) {
					this->hitFlag = true;
					this->hitTimer = 0;
					this->HitBullet(3);
					auto enemyPos = other->GetGameObject()->GetTransform()->GetWorldPosition();
					auto myPos = my->GetGameObject()->GetTransform()->GetWorldPosition();
					auto v = Math::Vector3::Subtract(myPos, enemyPos);
					v.y += 3;
					auto dir = Math::Vector3::Normalize(v);
					my->SetVelocity(dir, this->hitDistance);
				}
			}
		}
		}
	);
	this->physicsScene = this->server->GetPhysicsScene();
}

void KG::Component::SPlayerComponent::Update(float elapsedTime)
{
	if (!this->isActive) {
		this->Respawn(elapsedTime);
	}
	else {
		this->rotationTrasnform->SetRotation(this->inputs.rotation);
		//auto eulerInputs = KG::Math::Quaternion::ToEuler(this->inputs.rotation);
		//this->physics->AddTorque(XMFLOAT3(0, 1, 0), 40000);
		if (!hitFlag)
			this->ProcessMove(elapsedTime);

		this->hitTimer += elapsedTime;
		if (this->hitTimer >= this->hitInterval) {
			this->hitFlag = false;
			this->hitTimer = 0;
		}

	}
	packetSendTimer += elapsedTime;
	if (packetSendTimer > this->packetInterval)
	{
		this->SendSyncPacket();
		packetSendTimer = 0.0f;
	}
}

void KG::Component::SPlayerComponent::Respawn(float elapsedTime) {
	if (this->respawnTimer > 3.0f) {
		DirectX::XMFLOAT3 pos(10, 0, networkObjectId * 3);
		this->physics->SetPosition(pos);
		this->physics->SetVelocity(XMFLOAT3{ 0,0,0 }, 0);

		this->isActive = true;
		this->hpPoint = 5;
		this->respawnTimer = 0;
		this->hitFlag = false;
		this->hitTimer = 0;
	}
	else {
		this->respawnTimer += elapsedTime;
	}
}

void KG::Component::SPlayerComponent::Destroy()
{
	if (this->physics)
		this->physics->ReleaseActor();
	this->gameObject->Destroy();
}

void KG::Component::SPlayerComponent::SendSyncPacket()
{
	KG::Packet::SC_PLAYER_DATA syncPacket;
	syncPacket.position = this->trasnform->GetPosition();
	syncPacket.rotation = this->rotationTrasnform->GetRotation();
	syncPacket.forwardValue = this->forwardValue;
	syncPacket.rightValue = this->rightValue;
    syncPacket.playerHp = static_cast<float>(this->hpPoint) / static_cast<float>(this->MAX_HP);
	syncPacket.inputs.stateW = this->inputs.stateW;
	syncPacket.inputs.stateA = this->inputs.stateA;
	syncPacket.inputs.stateS = this->inputs.stateS;
	syncPacket.inputs.stateD = this->inputs.stateD;
	syncPacket.inputs.stateShift = this->inputs.stateShift;
	//std::cout << "SendSyncPacket f : " << forwardValue << " / r :" << rightValue << "\n";
	this->BroadcastPacket(&syncPacket);
}

void KG::Component::SPlayerComponent::ProcessMove(float elapsedTime)
{
	bool forwardInput = false;
	bool rightInput = false;
	float speed = IsTouching(this->inputs.stateShift) ? 10.0f : 6.0f;
	speed *= speedValue;
	if ( IsTouching(this->inputs.stateW) )
	{
		this->forwardValue += inputRatio * +1 * elapsedTime;
		forwardInput = true;
	}
	if ( IsTouching(this->inputs.stateS) )
	{
		this->forwardValue += inputRatio * -1 * elapsedTime;
		forwardInput = true;
	}
	if ( IsTouching(this->inputs.stateD) )
	{
		this->rightValue += inputRatio * +1 * elapsedTime;
		rightInput = true;
	}
	if ( IsTouching(this->inputs.stateA) )
	{
		this->rightValue += inputRatio * -1 * elapsedTime;
		rightInput = true;
	}

	if ( !forwardInput )
	{
		if ( abs(forwardValue) > this->inputMinimum )
		{
			forwardValue += inputRetRatio * ((forwardValue > 0) ? -1 : 1) * elapsedTime;
		}
		else
		{
			forwardValue = 0.0f;
		}
	}

	if ( !rightInput )
	{
		if ( abs(rightValue) > this->inputMinimum )
		{
			rightValue += inputRetRatio * ((rightValue > 0) ? -1 : 1) * elapsedTime;
		}
		else
		{
			rightValue = 0.0f;
		}
	}
	forwardValue = KG::Math::Clamp(forwardValue, -1.0f, 1.0f);
	rightValue = KG::Math::Clamp(rightValue, -1.0f, 1.0f);

	XMFLOAT3 rightVelo = XMFLOAT3(0,0,0);
	XMFLOAT3 forwardVelo = XMFLOAT3(0, 0, 0);
	if ( abs(this->forwardValue) >= this->inputMinimum )
	{
		rightVelo = Math::Vector3::Normalize(this->rotationTrasnform->GetWorldLook()) * (speed * this->forwardValue);
		//physics->AddForce(vec, 500 * speed * elapsedTime * this->forwardValue);
		//physics->SetVelocity(vec, 500 * speed * elapsedTime * this->forwardValue);
	}
	if ( abs(this->rightValue) >= this->inputMinimum )
	{

		forwardVelo = Math::Vector3::Normalize(this->rotationTrasnform->GetWorldRight()) * (speed * this->rightValue);
		//physics->SetVelocity(vec, 500 * speed * elapsedTime * this->rightValue);
		//physics->AddForce(vec, 500 * speed * elapsedTime * this->rightValue);
	}
	auto resultVector = rightVelo + forwardVelo;
	this->physics->Move(Math::Vector3::Normalize(resultVector), Math::Vector3::Length(resultVector));
}

bool KG::Component::SPlayerComponent::OnDrawGUI()
{
    ImGui::InputInt("HP", &this->hpPoint);
	return false;
}

bool KG::Component::SPlayerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	switch (type)
	{
	case KG::Packet::PacketType::CS_INPUT:
	{
		auto* InputPacket = KG::Packet::PacketCast<KG::Packet::CS_INPUT>(packet);
		this->inputs = *InputPacket;
	}
	return true;
	case KG::Packet::PacketType::CS_FIRE:
	{
		if (bulletCount > 0) {
			this->bulletCount -= 1;
			if (this->physicsScene) {
				this->server->LockWorld();
				auto* firePacket = KG::Packet::PacketCast<KG::Packet::CS_FIRE>(packet);
				KG::Packet::SC_FIRE returnPacket;
				returnPacket.direction = firePacket->direction;
				returnPacket.origin = firePacket->origin;
				returnPacket.distance = firePacket->distance;
				this->BroadcastPacket(&returnPacket);
				auto comp = this->physicsScene->QueryRaycast(firePacket->origin, firePacket->direction, firePacket->distance);
				if (comp) {
					auto callback = comp->GetRaycastCallback();
					if (callback) {
						callback(KG::Component::RaycastType::BULLET_HIT, this->physics);
					}
				}
				this->server->UnlockWorld();
			}
		}
	}
	return true;
	case KG::Packet::PacketType::CS_RELOAD:
	{
		this->bulletCount = 30;
	}
	return true;
	}
	return false;
}

void KG::Component::SPlayerComponent::HitBullet(int damage)
{
	hpPoint -= damage;
	if (hpPoint <= 0) {
		hpPoint = 0;
		isActive = false;
	}
}

