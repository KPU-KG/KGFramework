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

LightVertexOut VertexShaderFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    LightVertexOut result;
    
    float3 worldPosition = input.position * lightInfo[InstanceID].FalloffEnd * 1.1f;
    worldPosition = worldPosition + lightInfo[InstanceID].Position;
    
    result.position = mul(float4(worldPosition, 1), viewProjection);
    result.projPosition = result.position;
    result.InstanceID = InstanceID;
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
    float depth = InputGBuffer4.Sample(gsamPointWrap, uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy, mul(inverseProjection, inverseView));
    
    //float3 cameraDirection = look;
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    float3 lightDirection = calcWorldPosition - lightData.Position;
    
    float distance = length(lightDirection);
    
    float atten = CalcAttenuation(distance, lightData.FalloffStart, lightData.FalloffEnd);
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightDirection), normalize(-cameraDirection), atten);
}

