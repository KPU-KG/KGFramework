#include "GlobalDefine.hlsl"

float4 DefaultVertexFuction(VertexData input, uint InstanceID : SV_InstanceID) : SV_POSITION
{
    float4x4 wvp = mul(objectInfo[InstanceID].world, mul(view, projection));
    return mul(float4(input.position, 1), wvp);
}

float4 DefaultPixelFuction(float4 input : SV_POSITION, uint id : SV_PrimitiveID) : SV_TARGET
{
    return float4(id / 255.0f, 0, 0, 1);
}

