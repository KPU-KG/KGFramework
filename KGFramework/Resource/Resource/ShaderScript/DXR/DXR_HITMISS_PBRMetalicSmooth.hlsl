#include "DXR_HITMISS.hlsl"

// Write User Material Data Foramt
struct MaterialData
{
    uint ColorTextureIndex;
    uint NormalTextureIndex;
    uint MaskMapTextureIndex; // metalic / ao / detail mask / gloss
    float SpecularValue;
    float2 UVSize;
    float2 pad;
};

// Don't Touch This Line
StructuredBuffer<MaterialData> materialData : register(t3, space4);

// Write User Surface Shader Code
Surface UserSurfaceFunction(SurfaceInput input)
{
    Surface result;
    MaterialData mat = materialData[objectInfo[input.InstanceID].materialIndex];
    // MaterialData mat = materialData[objectInfo[0].materialIndex];

    float2x2 uvScale = float2x2(mat.UVSize.x, 0, 0, mat.UVSize.y);
    float2 uv = mul(input.uv, uvScale);
    result.albedo = GammaToLinear(shaderTexture[mat.ColorTextureIndex].SampleLevel(gsamAnisotoropicWrap, uv, 0).xyz);
    //result.reflection = objectInfo[input.InstanceID].environmentMapIndex / 12000.0f;

    result.specular = mat.SpecularValue;
    result.metalic = shaderTexture[mat.MaskMapTextureIndex].SampleLevel(gsamAnisotoropicWrap, uv, 0).r;
    result.roughness = 1 - shaderTexture[mat.MaskMapTextureIndex].SampleLevel(gsamAnisotoropicWrap, uv, 0).a;
     //result.roughness = 1.0f;
    result.emssion = 0.0f;

    float3x3 TBN = float3x3(
         normalize(input.worldTangent.xyz),
         normalize(input.worldBiTangent.xyz),
         normalize(input.worldNormal.xyz)
     );

    float3 normalMap = shaderTexture[mat.NormalTextureIndex].SampleLevel(gsamAnisotoropicWrap, uv, 0).xyz;
    normalMap = normalMap * 2 - 1.0f;
    result.wNormal = normalize(mul(normalMap, TBN));

    result.environmentMap = objectInfo[input.InstanceID].environmentMapIndex;
    result.reserve0 = 1.0f.xxx;

    return result;
}