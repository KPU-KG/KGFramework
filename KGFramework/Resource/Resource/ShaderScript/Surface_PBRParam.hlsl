#include "Define_Global.hlsl"
#include "Utill_ShaderSelector.hlsl"


// Write User Material Data Foramt
struct MaterialData
{
    float4 albedo;
    float roughness;
    float metalic;
    float specular;
    float pad;
};

// Don't Touch This Line
StructuredBuffer<MaterialData> materialData : register(t1);

// Write User Surface Shader Code
Surface UserSurfaceFunction(SurfaceInput input)
{
    Surface result;
    MaterialData mat = materialData[objectInfo[input.InstanceID].materialIndex];
    
    
    result.albedo = mat.albedo.rgb;
    result.specular = mat.specular;
    result.metalic = mat.metalic;
    result.roughness = mat.roughness;
    result.emssion = 0.0f;
    
    float3x3 TBN = float3x3(
        normalize(input.worldTangent.xyz),
        normalize(input.worldBiTangent.xyz),
        normalize(input.worldNormal.xyz)
    );
    
    float3 normalMap = float3(0,0,1);
    //normalMap = normalMap * 2 - 1.0f;
    result.wNormal = normalize(mul(normalMap, TBN));
    
    result.environmentMap = objectInfo[input.InstanceID].environmentMapIndex;
    result.reserve0 = 1.0f.xxx;
    
    return result;
}