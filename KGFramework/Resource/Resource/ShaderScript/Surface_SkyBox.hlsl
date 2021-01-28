#include "Define_Global.hlsl"
#include "Utill_ShaderSelector.hlsl"


// Write User Material Data Foramt
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

// Don't Touch This Line
StructuredBuffer<MaterialData> materialData : register(t1);

// Write User Surface Shader Code
Surface UserSurfaceFunction(SurfaceInput input)
{
    Surface result;
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
    
    float3 normalMap = shaderTexture[mat.NormalTextureIndex].Sample(gsamAnisotoropicWrap, uv).xyz;
    normalMap = normalMap * 2 - 1.0f;
    result.wNormal = normalize(mul(normalMap, TBN));
    
    result.environmentMap = objectInfo[input.InstanceID].environmentMapIndex;
    result.reserve0 = 1.0f.xxx;
    
    return result;
}