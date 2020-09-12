#include "GlobalDefine.hlsl"
#include "DeferredDefine.hlsl"
#include "GBufferDefine.hlsl"

struct MaterialData
{
    uint TextureIndex;
    float2 UVSize;
    float pad;
    float4 color;
};

StructuredBuffer<MaterialData> materialData : register(t1);

struct VSOutput
{
    float4 position : SV_Position;
    float4 worldPosition : WPOSITION;
    float4 worldNormal : WNORMAL;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

VSOutput DefaultVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    //float4x4 vp = viewProjection;
    float4x4 wvp = mul(objectInfo[InstanceID].world, viewProjection);
    //float4x4 wvp = mul(objectInfo[InstanceID].world, mul(view, projection));
    result.position = mul(float4(input.position, 1), wvp);
    result.worldPosition = mul(float4(input.position, 1), objectInfo[InstanceID].world);
    result.worldNormal = mul(float4(input.normal, 0), objectInfo[InstanceID].world);
    result.uv = input.uv;
    result.InstanceID = InstanceID; 
    return result;
}

GBufferOut DefaultPixelFuction(VSOutput input)
{
    PixelResult result;
    
    MaterialData mat = materialData[objectInfo[input.InstanceID].materialIndex];
    
    result.albedo = shaderTexture[mat.TextureIndex].Sample(gsamAnisotoropicWrap, input.uv).xyz * mat.color.xyz;
    result.reflection = 1.0f;
    
    result.specular = 0.0f;
    result.metalic = 1.0f;
    result.roughness = 0.25f;
    result.emssion = 0.0f;
    
    result.wNormal = normalize(input.worldNormal.xyz);
    
    result.light = float3(1, 0, 0);
    result.reserve0 = 1.0f;
    
    GBufferOut bufferResult = PixelEncode(result);
    return bufferResult;
}

