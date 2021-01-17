#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "AssimpImpoter.h"

#include "GameObject.h"
#include "Transform.h"

KG::Component::TransformComponent* KG::Core::GameObject::GetTransform() const
{
    return this->GetComponent<KG::Component::TransformComponent>();
}

KG::Core::GameObject* KG::Core::GameObject::FindChildObject( const KG::Utill::HashString& id ) const
{
    return nullptr;
}

/// //////////////////////////////
/////////////////////////////////
std::vector<std::pair<std::string, DirectX::XMFLOAT4X4>> KG::Core::GameObject::GetAnimationMatrix(double animationTime, int animationType) {
	// 애니메이션 듀레이션에 맞게 프레임 타임 조절
	double duration = animations[animationType]->mDuration;
	while (animationTime >= duration)
		animationTime -= duration;

	std::vector<std::pair<std::string, DirectX::XMFLOAT4X4>> output;

	for (unsigned int ch = 0; ch < animations[animationType]->mNumChannels; ++ch) {
		DirectX::XMFLOAT4X4 matrix;
		// position
		int posKey = 0;
		aiVector3D pos;
		aiVector3D prevPos;
		aiVector3D nextPos;
		double posT;
		unsigned int numPosKey = animations[animationType]->mChannels[ch]->mNumPositionKeys;
		for (; posKey < numPosKey; ++posKey) {
			if (animations[animationType]->mChannels[ch]->mPositionKeys[posKey].mTime > animationTime)
				break;
		}
		// posKey 가 마지막 키면 첫번째 키와 보간
		if (posKey == numPosKey) {
			double prevPosTime = animations[animationType]->mChannels[ch]->mPositionKeys[posKey - 1].mTime;
			posT = (animationTime - prevPosTime) / (duration - prevPosTime);
			prevPos = animations[animationType]->mChannels[ch]->mPositionKeys[posKey - 1].mValue;
			nextPos = animations[animationType]->mChannels[ch]->mPositionKeys[0].mValue;

		}
		else {
			double prevPosTime = animations[animationType]->mChannels[ch]->mPositionKeys[posKey].mTime;
			posT = (animationTime - prevPosTime) / (animations[animationType]->mChannels[ch]->mPositionKeys[posKey + 1].mTime - prevPosTime);
			prevPos = animations[animationType]->mChannels[ch]->mPositionKeys[posKey - 1].mValue;
			nextPos = animations[animationType]->mChannels[ch]->mPositionKeys[posKey].mValue;
		}
		pos.x = (1 - posT) * prevPos.x + posT * nextPos.x;
		pos.y = (1 - posT) * prevPos.y + posT * nextPos.y;
		pos.z = (1 - posT) * prevPos.z + posT * nextPos.z;

		// rotation
		int rotKey = 0;
		aiQuaternion rot;
		aiQuaternion prevRot;
		aiQuaternion nextRot;
		double rotT;
		unsigned int numRotKey = animations[animationType]->mChannels[ch]->mNumRotationKeys;
		for (; rotKey < numRotKey; ++rotKey) {
			if (animations[animationType]->mChannels[ch]->mRotationKeys[rotKey].mTime > animationTime)
				break;
		}

		if (rotKey == numRotKey) {
			double prevRotTime = animations[animationType]->mChannels[ch]->mRotationKeys[rotKey - 1].mTime;
			rotT = (animationTime - prevRotTime) / (duration - prevRotTime);
			prevRot = animations[animationType]->mChannels[ch]->mRotationKeys[rotKey - 1].mValue;
			nextRot = animations[animationType]->mChannels[ch]->mRotationKeys[0].mValue;
		}
		else {
			double prevRotTime = animations[animationType]->mChannels[ch]->mRotationKeys[rotKey - 1].mTime;
			rotT = (animationTime - prevRotTime) / (animations[animationType]->mChannels[ch]->mRotationKeys[posKey + 1].mTime - prevRotTime);
			prevRot = animations[animationType]->mChannels[ch]->mRotationKeys[rotKey - 1].mValue;
			nextRot = animations[animationType]->mChannels[ch]->mRotationKeys[rotKey].mValue;
		}
		aiQuaternion::Interpolate(rot, prevRot, nextRot, rotT);

		// scale
		int scaleKey = 0;
		aiVector3D scale;
		aiVector3D prevScale;
		aiVector3D nextScale;
		double scaleT;
		unsigned int numScaleKey = animations[animationType]->mChannels[ch]->mNumScalingKeys;
		for (; scaleKey < numScaleKey; ++scaleKey) {
			if (animations[animationType]->mChannels[ch]->mScalingKeys[scaleKey].mTime > animationTime)
				break;
		}
		if (scaleKey == numScaleKey) {
			double prevScaleTime = animations[animationType]->mChannels[ch]->mScalingKeys[scaleKey - 1].mTime;
			scaleT = (animationTime - prevScaleTime) / (duration - prevScaleTime);
			prevScale = animations[animationType]->mChannels[ch]->mScalingKeys[scaleKey - 1].mValue;
			nextScale = animations[animationType]->mChannels[ch]->mScalingKeys[0].mValue;

		}
		else {
			double prevScaleTime = animations[animationType]->mChannels[ch]->mScalingKeys[scaleKey - 1].mTime;
			scaleT = (animationTime - prevScaleTime) / (animations[animationType]->mChannels[ch]->mScalingKeys[posKey + 1].mTime - prevScaleTime);
			prevPos = animations[animationType]->mChannels[ch]->mScalingKeys[scaleKey - 1].mValue;
			nextPos = animations[animationType]->mChannels[ch]->mScalingKeys[scaleKey].mValue;
		}
		scale.x = (1 - scaleT) * prevScale.x + scaleT * nextScale.x;
		scale.y = (1 - scaleT) * prevScale.y + scaleT * nextScale.y;
		scale.z = (1 - scaleT) * prevScale.z + scaleT * nextScale.z;


		DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));

		aiMatrix3x3 rm = rot.GetMatrix();
		DirectX::XMFLOAT4X4 rotation = {
			 rm.a1, rm.a2, rm.a3, 0,
			 rm.b1, rm.b2, rm.b3, 0,
			 rm.c1, rm.c2, rm.c3, 0,
			 0, 0, 0, 1
		};

		DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&matrix), DirectX::XMLoadFloat4x4(&rotation)));
		DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&matrix), DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z)));
		std::string name;
		name.append(animations[animationType]->mChannels[ch]->mNodeName.C_Str());
		output.emplace_back(std::make_pair(name, matrix));
	}

	return output;
	///////////////////////////////////////////////////
}