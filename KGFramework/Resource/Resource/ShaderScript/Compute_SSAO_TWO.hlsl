#include "Define_Compute.hlsl"
#include "Compute_SSAO_DEFINE.hlsl"


groupshared float sharedDepth[1024];

[numthreads(1024, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 res = GetRes();
    float2 resRcp = GetResRcp();
    uint2 CurPixel = uint2(dispatchThreadID.x % res.x, dispatchThreadID.x / res.x);
    sharedDepth[groupThreadID.x] = buffer0[GetPositionBuffer(dispatchThreadID.x)].x;
    GroupMemoryBarrierWithGroupSync();
    
    if (CurPixel.y < res.y)
    {
        float2 centerClipPos = 2.0f * float2(CurPixel) * resRcp;
        centerClipPos = float2(centerClipPos.x - 1.0f, 1.0 - centerClipPos.y);
        buffer1[CurPixel] = ComputeAO(CurPixel, centerClipPos, GetProjValue(projection));
    }
}