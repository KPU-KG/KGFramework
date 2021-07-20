#include "Define_Compute.hlsl"


groupshared float shaderCache[8][8];
[numthreads(1, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    
    float count = 0.0f;
    if (dispatchThreadID.x == 0 && dispatchThreadID.y == 0)
    {
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                shaderCache[x][y] = 0.0f;
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();
    shaderCache[groupId.x][groupId.y] = buffer0.Load(int2(groupId.x * 512 + groupId.x, groupId.y * 512 + groupId.y)).x;
    GroupMemoryBarrierWithGroupSync();
    if (dispatchThreadID.x == 0 && dispatchThreadID.y == 0)
    {
        float lum = 0.0f;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                lum += shaderCache[x][y];
            }
        }
        lum /= (ceil(texWidth / 512.0f) * ceil(texHeight / 512.0f));
        buffer1[int2(0, 0)] = lerp(buffer1[int2(0, 0)], lum, 0.05f);
        //buffer1[int2(0, 0)] = lum;
    }
}