#ifndef __LIGHT_DEFINE__
#define __LIGHT_DEFINE__

struct LightData
{
    float3 Strength;
    float FalloffStart;
    float3 Direction;
    float FalloffEnd;
    float3 Position;
    float SpotPower;
    uint shadowMapIndex;
    float3 padding0;
    float4 padding1;
};

StructuredBuffer<LightData> lightInfo : register(t0);

Texture2D InputGBuffer0 : register(t2);
Texture2D InputGBuffer1 : register(t3);
Texture2D InputGBuffer2 : register(t4);
Texture2D<uint4> InputGBuffer3 : register(t5);
Texture2D InputGBuffer4 : register(t6);

Texture2D<float> shadowTexture[] : register(t0, space1);
TextureCube<float> shadowCubeTexture[] : register(t0, space1);

#endif