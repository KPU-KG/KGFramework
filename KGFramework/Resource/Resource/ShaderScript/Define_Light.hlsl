#ifndef __LIGHT_DEFINE__
#define __LIGHT_DEFINE__

struct LightData
{
    float3 Strength;
    float lightPower;
    float3 Direction;
    float pad0;
    float3 Position;
    float pad1;
    float FalloffStart;
    float FalloffEnd;
    float Phi;
    float Theta;
    float3 Up;
    float pad2;
};

StructuredBuffer<LightData> lightInfo : register(t0);

struct ShadowData
{
    uint shadowMapIndex[4];
    float4x4 shadowMatrix[4];
};
StructuredBuffer<ShadowData> shadowInfo : register(t0, space4);

Texture2D InputGBuffer0 : register(t2);
Texture2D InputGBuffer1 : register(t3);
Texture2D InputGBuffer2 : register(t4);
Texture2D<uint4> InputGBuffer3 : register(t5);
Texture2D InputGBuffer4 : register(t6);

float3 DepthToWorldPosition(float depth, float2 projPosition, float4x4 inverseViewProj)
{
    float4 projPos = float4(projPosition.x, projPosition.y, depth, 1.0f);
    float4 worldPos = mul(projPos, inverseViewProj);
    return worldPos.xyz / worldPos.w;
}

float2 ProjPositionToUV(float2 projPosition)
{
    projPosition.x += 1.0f;
    projPosition.y += 1.0f;
    projPosition.x *= 0.5f;
    projPosition.y *= 0.5f;
    projPosition.y = 1 - projPosition.y;
    return projPosition;
}

float2 UVToProjPosition(float2 projPosition)
{
    projPosition.x -= 1.0f;
    projPosition.y -= 1.0f;
    projPosition.x /= 0.5f;
    projPosition.y /= 0.5f;
    projPosition.y = 1 - projPosition.y;
    return projPosition;
}

#endif