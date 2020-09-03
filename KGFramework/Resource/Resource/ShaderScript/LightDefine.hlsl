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
    float4 padding0;
    uint padding1;
};

StructuredBuffer<LightData> lightInfo : register(t0);

Texture2D InputGBuffer0 : register(t2);
Texture2D InputGBuffer1 : register(t3);
Texture2D InputGBuffer2 : register(t4);
Texture2D InputGBuffer3 : register(t5);
Texture2D InputGBuffer4 : register(t6);

#endif