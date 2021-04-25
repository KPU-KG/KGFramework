#include "pch.h"
#include "ServerPlayerControllerComponent.h"
#include "Transform.h"
#include "KGServer.h"

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
}

void KG::Component::SPlayerComponent::Update(float elapsedTime)
{
	this->ProcessMove(elapsedTime);
	this->trasnform->SetRotation(this->inputs.rotation);

	packetSendTimer += elapsedTime;
	if ( packetSendTimer > this->packetInterval )
	{
		this->SendSyncPacket();
		packetSendTimer = 0.0f;
	}
}

void KG::Component::SPlayerComponent::SendSyncPacket()
{
	KG::Packet::SC_PLAYER_DATA syncPacket;
	syncPacket.position = this->trasnform->GetPosition();
	syncPacket.rotation = this->trasnform->GetRotation();
	syncPacket.forwardValue = this->forwardValue;
	syncPacket.rightValue = this->rightValue;
	std::cout << "SendSyncPacket f : " << forwardValue << " / r :" << rightValue << "\n";
	this->BroadcastPacket(&syncPacket);
}

void KG::Component::SPlayerComponent::ProcessMove(float elapsedTime)
{
	bool forwardInput = false;
	bool rightInput = false;
	float speed = IsTouching(this->inputs.stateShift) ? 6.0f : 2.0f;
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

	if ( abs(this->forwardValue) >= this->inputMinimum )
	{
		this->trasnform->Translate(this->trasnform->GetLook() * speed * elapsedTime * this->forwardValue);
	}
	if ( abs(this->rightValue) >= this->inputMinimum )
	{
		this->trasnform->Translate(this->trasnform->GetRight() * speed * elapsedTime * this->rightValue);
	}
}

bool KG::Component::SPlayerComponent::OnDrawGUI()
{
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
	}
	return false;
}

