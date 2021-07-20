#include "Define_Compute.hlsl"

[numthreads(4, 4, 1)]
void ComputeShaderFunction(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    
    int texPosX = clamp(dispatchThreadID.x, 0, texWidth - 1);
    int texPosY = clamp(dispatchThreadID.y, 0, texHeight - 1);
    
    float4 color = prevResult[int2(texPosX, texPosY)];
    outputResult[dispatchThreadID.xy] = color;
}