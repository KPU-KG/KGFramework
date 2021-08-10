#include "Define_Compute.hlsl"

[numthreads(32, 32, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    if (dispatchThreadID.x < texWidth && dispatchThreadID.y < texHeight)
    {
        float3 result = prevResult[dispatchThreadID.xy];
        outputResult[dispatchThreadID.xy] = float4(LinearToGamma(result), 1.0f);
    }
}