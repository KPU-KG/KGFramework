#include "Define_Compute.hlsl"

[numthreads(32, 32, 1)]
void ComputeShaderFunction(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    int texLevel = -1;
    inputSource.GetDimensions(0, texWidth, texHeight, texLevel);
    
    int texPosX = clamp(dispatchThreadID.x, 0, texWidth - 1);
    int texPosY = clamp(dispatchThreadID.y, 0, texHeight - 1);
    //sharedCache[groupThreadID.x][groupThreadID.y][groupThreadID.z] = inputSource[int2(texPosX, texPosY)];
    
    ////Sync
    //GroupMemoryBarrierWithGroupSync();
    float4 color = prevResult[int2(texPosX, texPosY)];
    float4 filter = float4(0.299, 0.587, 0.114, 0);
    outputResult[dispatchThreadID.xy] = dot(color, filter);

}