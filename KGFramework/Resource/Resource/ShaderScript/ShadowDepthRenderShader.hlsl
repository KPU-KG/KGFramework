#include "GlobalDefine.hlsl"
#include "DeferredDefine.hlsl"

struct VSOutput
{
    float4 position : SV_Position;
};

float4 DepthVertexFunction(VertexData input, uint InstanceID : SV_InstanceID) : SV_Position
{
    float4x4 wvp = mul(objectInfo[InstanceID].world, viewProjection);
    return mul(float4(input.position, 1), wvp);
}

float4 DepthPixelFunction(float4 input : SV_Position) : SV_Target0
{
    return float4(0, 0, 0, 0);
}

