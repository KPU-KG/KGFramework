#include <DirectXMath.h>
#include "IParticleEmitterComponent.h"
#include "MathHelper.h"

using namespace KG::Math::Literal;

int KG::Component::ParticleDesc::GetCount() const
{
    return this->baseEmitCount + KG::Math::RandomAbsRangeInt(this->rangeEmitCount);
}

KG::Component::ParticleData KG::Component::ParticleDesc::GetData(const DirectX::XMFLOAT3& position) const
{
    ParticleData data;
    DirectX::XMFLOAT3 deltaPosition = KG::Math::RandomVector3(baseDeltaPosition - rangeDeltaPosition, baseDeltaPosition + rangeDeltaPosition);
    data.position = position + deltaPosition;
    data.size = KG::Math::RandomVector2(baseSize - rangeSize, baseSize + rangeSize);
    data.speed = KG::Math::RandomVector3(baseSpeed - rangeSpeed, baseSpeed + rangeSpeed);
    data.color = color.ToFloat4();
    data.lifeTime = KG::Math::RandomFloat(baselifeTime - rangelifeTime, baselifeTime + rangelifeTime);
    data.rotation = KG::Math::RandomFloat(baseRotation - rangeRotation, baseRotation + rangeRotation);
    data.rotationSpeed = KG::Math::RandomFloat(baseRotationSpeed - rangeRotationSpeed, baseRotationSpeed + rangeRotationSpeed);
    data.materialIndex = this->materialIndex;
    return data;
}

KG::Component::ParticleData KG::Component::ParticleDesc::GetData(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& baseSpeed) const
{
    ParticleData data;
    DirectX::XMFLOAT3 deltaPosition = KG::Math::RandomVector3(baseDeltaPosition - rangeDeltaPosition, baseDeltaPosition + rangeDeltaPosition);
    data.position = position + deltaPosition;
    data.size = KG::Math::RandomVector2(baseSize - rangeSize, baseSize + rangeSize);
    data.speed = baseSpeed + KG::Math::RandomVector3(baseSpeed - rangeSpeed, baseSpeed + rangeSpeed);
    data.color = color.ToFloat4();
    data.lifeTime = KG::Math::RandomFloat(baselifeTime - rangelifeTime, baselifeTime + rangelifeTime);
    data.rotation = KG::Math::RandomFloat(baseRotation - rangeRotation, baseRotation + rangeRotation);
    data.rotationSpeed = KG::Math::RandomFloat(baseRotationSpeed - rangeRotationSpeed, baseRotationSpeed + rangeRotationSpeed);
    data.materialIndex = this->materialIndex;
    return data;
}
