#include "Define_Global.hlsl"

float4x4 GetWorldMatrix(uint instanceID, VertexData vertex)
{
    return objectInfo[instanceID].world;
}