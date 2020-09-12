#include "GlobalDefine.hlsl"
#include "DeferredDefine.hlsl"
#include "GBufferDefine.hlsl"

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


SkyBoxOut SKYMAP_VS(VertexData vIn)
{
    SkyBoxOut vOut;
    vOut.PosL = vIn.position;
    float4 posW = float4(vIn.position + cameraWorldPosition, 1.0f);
    
    vOut.PosH = mul(posW, viewProjection).xyww;
    return vOut;
}

float4 SKYMAP_PS(SkyBoxOut pIn) : SV_Target
{
    MaterialData mat = materialData[0];
    return shaderTextureCube[mat.SkyBoxTextureIndex].Sample(gsamAnisotoropicWrap, pIn.PosL);
}


