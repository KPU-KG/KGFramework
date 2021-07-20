#include "Define_Compute.hlsl"
#define UNIT_COUNT 1024

groupshared float4 shaderCache[UNIT_COUNT];
static const float4 lumFactor = float4(0.299, 0.587, 0.114, 0);

[numthreads(1, UNIT_COUNT, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    int texLevel = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    
    int texPosX = clamp(dispatchThreadID.x, 0, texWidth - 1);
    int texPosY = clamp(dispatchThreadID.y, 0, texHeight - 1);
    
    shaderCache[groupThreadID.y] = prevResult.Load(dispatchThreadID.xy);
    GroupMemoryBarrierWithGroupSync();
    float lum;
    float count = 0;
    for (int i = 0; i < UNIT_COUNT; i++)
    {
        lum += dot(shaderCache[i], lumFactor);
    }
    lum /= clamp((texHeight - groupId.y * UNIT_COUNT), 1, UNIT_COUNT);
    buffer0[dispatchThreadID.xy] = float4(lum.xxxx);
    outputResult[dispatchThreadID.xy] = shaderCache[groupThreadID.y];
}