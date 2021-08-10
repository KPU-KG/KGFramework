#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"

// Write User Material Data Foramt
struct MaterialData
{
    uint ColorTextureIndex;
    float alpha;
};

// Don't Touch This Line
StructuredBuffer<MaterialData> materialData : register(t1);

struct TransparentVertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct TransparentVertexOutput
{
    float4 position : SV_Position;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

TransparentVertexOutput VertexShaderFunction(TransparentVertexInput input, uint InstanceID : SV_InstanceID)
{
    TransparentVertexOutput result;
    result.positionW = mul(input.position, objectInfo[InstanceID].world);
    result.position = mul(float4(result.positionW, 1), viewProjection);
    result.normal = float3(0, 0, 0);
    result.uv = input.uv;
    result.InstanceID = InstanceID;
    return result;
}

float4 PixelShaderFunction(TransparentVertexOutput input) : SV_Target0
{
    MaterialData mat = materialData[objectInfo[input.InstanceID].materialIndex];
    float4 color = shaderTexture[mat.ColorTextureIndex].Sample(gsamAnisotoropicClamp, input.uv);
    color.rgb = GammaToLinear(color.rgb);
    color.a = color.a * mat.alpha;
    return color;
}

