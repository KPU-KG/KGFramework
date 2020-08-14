#include "GlobalDefine.hlsl"
#include "DeferredDefine.hlsl"

struct VSOutput
{
    float4 position : SV_Position;
    float4 worldPosition : WPOSITION;
    float4 worldNormal : WNORMAL;
    float2 uv : TEXCOORD;
};

VSOutput DefaultVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    
    float4x4 wvp = mul(objectInfo[InstanceID].world, mul(view, projection));
    result.position = mul(float4(input.position, 1), wvp);
    result.worldPosition = mul(float4(input.position, 1), objectInfo[InstanceID].world);
    result.worldNormal = mul(float4(input.normal, 0), objectInfo[InstanceID].world);
    result.uv = input.uv;
    return result;
}

GBufferOut DefaultPixelFuction(VSOutput input)
{
    PixelResult result;
    
    result.diffuse = float3(1, 0, 0);
    result.reserve0 = 1;
    
    result.specular = float3(0, 1, 0);
    result.roughness = 1;
    
    result.wNormal = normalize(input.worldNormal).xyz;
    result.reflection = 1;
    
    result.light = float3(0, 0, 0);
    result.emission = 1;
    
    return PixelEncode(result);
}

