#ifndef __COMPUTE_DEFINE__
#define __COMPUTE_DEFINE__
#include "Define_Global.hlsl"
#include "Define_GBuffer.hlsl"

RWTexture2D<float4> outputResult : register(u0);
Texture2D<float4> prevResult : register(t0);
Texture2D inputSource : register(t0, space1);
Texture2D InputGBuffer0 : register(t1, space1);
Texture2D InputGBuffer1 : register(t2, space1);
Texture2D InputGBuffer2 : register(t3, space1);
Texture2D<uint4> InputGBuffer3 : register(t4, space1);
Texture2D InputGBuffer4 : register(t5, space1);

RWTexture2D<float4> buffer0 : register(u0, space2);
RWTexture2D<float4> buffer1 : register(u1, space2);
RWTexture2D<float4> buffer2 : register(u2, space2);

Texture2D<float4> prevBuffer0 : register(t0, space3);
Texture2D<float4> prevBuffer1 : register(t1, space3);
Texture2D<float4> prevBuffer2 : register(t2, space3);

RWTexture2D<float4> frameData : register(u0, space3);

cbuffer CameraData : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
	
    float4x4 inverseView;
    float4x4 inverseProjection;
    float4x4 inverseViewProjection;
	
    float3 cameraWorldPosition;
    float1 pad0;
    float3 look;
    float1 pad1;
    double gameTime;
};

cbuffer Material : register(b0, space1)
{
    uint materialIndex;
};


#endif // __DEPENDENCY_HLSL__
