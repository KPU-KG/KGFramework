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
    
    float4x4 animationMatrix = (float4x4) 0.0f;
    for (int i = 0; i < 4; i++)
    {
        int animationIndex = (MAX_COUNT_BONE * InstanceID) + input.bone[i];
        float4x4 bone = boneOffsetInfo[input.bone[i]];
        float4x4 anim = animationTransformInfo[animationIndex];
        float4x4 vTobone = mul(bone, anim);
        animationMatrix += input.weight[i] * vTobone;
    }
    float4x4 world = animationMatrix;
    result.worldPosition = mul(float4(input.position, 1), world);
    result.position = mul(result.worldPosition, viewProjection);
    result.worldNormal = mul(float4(input.normal, 0), animationMatrix);
    result.worldTangent = mul(float4(input.tangent, 0), animationMatrix);
    result.worldBiTangent = mul(float4(input.bitangent, 0), animationMatrix);
    
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

