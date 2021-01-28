#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"
#include "Define_Light.hlsl"
#include "Utill_LightCustom.hlsl"

struct MaterialData
{
    uint SkyBoxTextureIndex;
};

StructuredBuffer<MaterialData> materialData : register(t1);

struct SkyBoxOut
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

SkyBoxOut VertexShaderFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    SkyBoxOut vOut;
    vOut.PosL = input.position;
    float4 posW = float4(input.position + cameraWorldPosition, 1.0f);
    vOut.PosH = mul(posW, viewProjection).xyww;
    return vOut;
}

float4 PixelShaderFuction(SkyBoxOut input) : SV_Target0
{
    MaterialData mat = materialData[0];
    return shaderTextureCube[mat.SkyBoxTextureIndex].Sample(gsamAnisotoropicWrap, normalize(input.PosL));
}

