#include "Define_Compute.hlsl"

groupshared float shaderCache;

static const float3 lumFactor = float3(0.299, 0.587, 0.114);
static const float MiddleGray = 0.229f;
static const float LimWhiteSQR = 5.8f;

float3 ToneMapping(float3 hdrColor)
{
    float LScale = dot(hdrColor, lumFactor);
    LScale *= MiddleGray / shaderCache;
    LScale = (LScale + LScale * LScale / (LimWhiteSQR)) / (1.0 + LScale);
    return hdrColor * LScale;
}

[numthreads(32, 32, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    shaderCache = buffer1.Load(int3(0, 0, 0)).x;
    GroupMemoryBarrierWithGroupSync();
    float3 color = ToneMapping(prevResult[dispatchThreadID.xy].xyz);
    outputResult[dispatchThreadID.xy] = float4(color.x, color.y, color.z, 1.0f);
}