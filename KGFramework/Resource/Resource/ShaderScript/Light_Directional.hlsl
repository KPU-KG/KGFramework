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

Texture2DArray<float> shadowArray[] : register(t0, space1);


float DirectionalShadowCascadePCF(float3 worldPosition, LightData lightData, ShadowData shadowData)
{
    float4 projPos = mul(float4(worldPosition, 1.0f), shadowData.shadowMatrix[0]);
    float3 projPos3 = projPos.xyz / projPos.w;
    float2 uv = ProjPositionToUV(projPos3.xy);
    return shadowArray[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamLinerCompClamp, float3(uv, 0), (projPos3.z) - 0.001f);
}


LightVertexOut VertexShaderFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    LightVertexOut result;
    result.position = float4(input.position, 1.0f);
    result.InstanceID = InstanceID;
    result.projPosition = result.position;
    return result;
}

float4 PixelShaderFuction(LightVertexOut input) : SV_Target0
{
    input.projPosition /= input.projPosition.w;
    float2 uv = ProjPositionToUV(input.projPosition.xy);
    Surface pixelData = PixelDecode(
        InputGBuffer0.Sample(gsamPointWrap, uv),
        InputGBuffer1.Sample(gsamPointWrap, uv),
        InputGBuffer2.Sample(gsamPointWrap, uv),
        InputGBuffer3.Sample(gsamPointWrap, uv)
    );
    ShadowData shadowData = shadowInfo[input.InstanceID];

    float depth = InputGBuffer4.Sample(gsamPointWrap, uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy, mul(inverseProjection, inverseView));
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    
    float shadowFactor = DirectionalShadowCascadePCF(calcWorldPosition, lightData, shadowData);
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightData.Direction), normalize(-cameraDirection), 1.0f) * shadowFactor;
}

