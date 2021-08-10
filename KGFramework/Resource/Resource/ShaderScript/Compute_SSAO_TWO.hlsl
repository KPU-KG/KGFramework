#include "Define_Compute.hlsl"
#include "Compute_SSAO_DEFINE.hlsl"


groupshared float sharedDepth[1024];

[numthreads(1024, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 res = GetRes();
    float2 resRcp = GetResRcp();
    uint2 CurPixel = uint2(dispatchThreadID.x % res.x, dispatchThreadID.x / res.x);
    sharedDepth[groupThreadID.x] = prevBuffer0[CurPixel.xy].x;
    GroupMemoryBarrierWithGroupSync();
    
    if (CurPixel.y < res.y)
    {
        float2 centerClipPos = 2 * float2(CurPixel) * resRcp;
        centerClipPos = float2(centerClipPos.x - 1.0f, 1.0 - centerClipPos.y);
        float value = ComputeAO(CurPixel, centerClipPos, GetProjValue(projection));
        buffer0[CurPixel] = value;
        //buffer2[CurPixel] = value;
        uint2 newCurPixel = uint2(CurPixel.x * 2, CurPixel.y * 2);
        
        buffer1[newCurPixel + uint2(0, 0)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(0, 1)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(0, 2)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(0, 3)] = float4(value.xxx, 1);
        
        buffer1[newCurPixel + uint2(1, 0)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(1, 1)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(1, 2)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(1, 3)] = float4(value.xxx, 1);
        
        buffer1[newCurPixel + uint2(2, 0)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(2, 1)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(2, 2)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(2, 3)] = float4(value.xxx, 1);
        
        buffer1[newCurPixel + uint2(3, 0)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(3, 1)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(3, 2)] = float4(value.xxx, 1);
        buffer1[newCurPixel + uint2(3, 3)] = float4(value.xxx, 1);
        //
        buffer2[newCurPixel + uint2(0, 0)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(0, 1)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(0, 2)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(0, 3)] = float4(0.25f.xxx, 1);
        
        buffer2[newCurPixel + uint2(1, 0)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(1, 1)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(1, 2)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(1, 3)] = float4(0.25f.xxx, 1);
        
        buffer2[newCurPixel + uint2(2, 0)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(2, 1)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(2, 2)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(2, 3)] = float4(0.25f.xxx, 1);
        
        buffer2[newCurPixel + uint2(3, 0)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(3, 1)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(3, 2)] = float4(0.25f.xxx, 1);
        buffer2[newCurPixel + uint2(3, 3)] = float4(0.25f.xxx, 1);
    }
}