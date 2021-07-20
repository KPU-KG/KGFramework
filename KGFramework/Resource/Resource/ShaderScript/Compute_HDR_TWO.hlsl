#include "Define_Compute.hlsl"
#define UNIT_COUNT 1024

groupshared float shaderCache[UNIT_COUNT];

[numthreads(UNIT_COUNT, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    int texLevel = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    
    int texPosX = clamp(dispatchThreadID.x, 0, texWidth - 1);
    int texPosY = clamp(dispatchThreadID.y, 0, texHeight - 1);
    
    shaderCache[groupThreadID.x] = buffer0.Load(dispatchThreadID.xy).x;
    GroupMemoryBarrierWithGroupSync();
    float lum = 0;
    float count = 0;
    for (int i = 0; i < UNIT_COUNT; i++)
    {
        lum += shaderCache[i];
    }
    lum /= clamp((texWidth - groupId.x * UNIT_COUNT), 1, UNIT_COUNT);
    buffer0[dispatchThreadID.xy] = float4(lum.xxxx);
    outputResult[dispatchThreadID.xy] = prevResult.Load(dispatchThreadID.xy);
}