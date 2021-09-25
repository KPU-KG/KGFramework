#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"
#include "Define_Light.hlsl"
#include "Utill_LightCustom.hlsl"

struct LightVertexOut
{
    float4 position : SV_Position;
    float4 projPosition : POSITION;
    uint InstanceID : SV_InstanceID;
};

struct MaterialData
{
    uint lutTextureIndex;
    uint irradianceIndex;
    uint radianceIndex;
    uint pad;
};

StructuredBuffer<MaterialData> materialData : register(t1);

LightVertexOut VertexShaderFunction(VertexData input, uint InstanceID : SV_InstanceID)
{
    LightVertexOut result;
    result.position = float4(input.position, 1.0f);
    result.InstanceID = InstanceID;
    result.projPosition = result.position;
    return result;
}

float4 PixelShaderFunction(LightVertexOut input) : SV_Target0
{
    input.projPosition /= input.projPosition.w;
    float2 uv = ProjPositionToUV(input.projPosition.xy);
    Surface pixelData = PixelDecode(
    InputGBuffer0.Sample(gsamPointWrap, uv),
    InputGBuffer1.Sample(gsamPointWrap, uv),
    InputGBuffer2.Sample(gsamPointWrap, uv),
    InputGBuffer3.Load(input.position)
    );

    float depth = InputGBuffer4.Sample(gsamPointWrap, uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy, mul(inverseProjection, inverseView));
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    //saturate(pixelData.ao)
    return CustomAmbientLightCalculator(lightData, pixelData, normalize(lightData.Direction), normalize(cameraDirection), 1.0f,
    materialData[0].lutTextureIndex, materialData[0].irradianceIndex, materialData[0].radianceIndex);
}

