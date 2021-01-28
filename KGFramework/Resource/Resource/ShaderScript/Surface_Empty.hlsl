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
    return result;
}