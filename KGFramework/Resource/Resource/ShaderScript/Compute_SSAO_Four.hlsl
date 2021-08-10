#include "Define_Compute.hlsl"
#include "Compute_SSAO_DEFINE.hlsl"
#include "Compute_Blur_Define.hlsl"

[numthreads(groupThreads, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID, uint gi : SV_GroupIndex)
{
    HorizontalFilter(groupId, gi, prevBuffer0, buffer0);
}