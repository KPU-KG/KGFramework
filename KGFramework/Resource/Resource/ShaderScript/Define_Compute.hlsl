#ifndef __COMPUTE_DEFINE__
#define __COMPUTE_DEFINE__
#include "Define_Global.hlsl"
#include "Define_GBuffer.hlsl"

RWTexture2D<float4> outputResult : register(u0);
RWTexture2D<float4> prevResult : register(u1);
Texture2D inputSource : register(t0, space1);
Texture2D InputGBuffer0 : register(t1, space1);
Texture2D InputGBuffer1 : register(t2, space1);
Texture2D InputGBuffer2 : register(t3, space1);
Texture2D<uint4> InputGBuffer3 : register(t4, space1);
Texture2D InputGBuffer4 : register(t5, space1);

#endif // __DEPENDENCY_HLSL__
