#include "GlobalDefine.hlsl"
#include "DeferredDefine.hlsl"
#include "GBufferDefine.hlsl"

struct MaterialData
{
    uint ColorTextureIndex;
    uint NormalTextureIndex;
    uint MetalicTextureIndex;
    uint RoughnessTextureIndex;
    float SpecularValue;
    float2 UVSize;
    float pad;
};

StructuredBuffer<MaterialData> materialData : register(t1);

struct VSOutput
{
    float4 position : SV_Position;
    float4 worldPosition : WPOSITION;
    float4 worldNormal : WNORMAL;
    float4 worldTangent : WTANGENT;
    float4 worldBiTangent : WBITANGENT;
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
    //result.worldNormal = float4(input.normal, 0);
    //result.worldTangent = float4(input.tangent, 0);
    //result.worldBiTangent = float4(input.bitangent, 0);
    
    result.worldNormal = mul(float4(input.normal, 0), objectInfo[InstanceID].world);
    result.worldTangent = mul(float4(input.tangent, 0), objectInfo[InstanceID].world);
    result.worldBiTangent = mul(float4(input.bitangent, 0), objectInfo[InstanceID].world);
    
    result.uv = input.uv;
    result.InstanceID = InstanceID;
    return result;
}

GBufferOut DefaultPixelFuction(VSOutput input)
{
    PixelResult result;
    MaterialData mat = materialData[objectInfo[input.InstanceID].materialIndex];
    
    float2x2 uvScale = float2x2(mat.UVSize.x, 0, 0, mat.UVSize.y);
    float2 uv = mul(input.uv, uvScale);
    
    
    result.albedo = shaderTexture[mat.ColorTextureIndex].Sample(gsamAnisotoropicWrap, uv).xyz;
    result.reflection = objectInfo[input.InstanceID].environmentMapIndex / 12000.0f;
    
    result.specular = 0.0f;
    result.metalic = shaderTexture[mat.MetalicTextureIndex].Sample(gsamAnisotoropicWrap, uv).xxx;
    result.roughness = shaderTexture[mat.RoughnessTextureIndex].Sample(gsamAnisotoropicWrap, uv).xxx;
    result.emssion = 0.0f;
    
    float3x3 TBN = float3x3(
        normalize(input.worldTangent.xyz),
        normalize(input.worldBiTangent.xyz),
        normalize(input.worldNormal.xyz)
    );
    //TBN = transpose(TBN);
    float3 normalMap = shaderTexture[mat.NormalTextureIndex].Sample(gsamAnisotoropicWrap, uv).xyz;
    normalMap = normalMap * 2 - 1.0f;
    result.wNormal = normalize(mul(normalMap, TBN));
    //result.wNormal = input.worldNormal;
    
    result.environmentMap = objectInfo[input.InstanceID].environmentMapIndex;
    result.reserve0 = 1.0f.xxx;
    
    GBufferOut bufferResult = PixelEncode(result);
    return bufferResult;
}

