#include "GlobalDefine.hlsl"
#include "GBufferDefine.hlsl"
#include "LightDefine.hlsl"

#include "CustomLight.hlsl"

struct VSOutput
{
    float4 position : SV_Position;
    float4 projPosition : POSITION;
    uint InstanceID : SV_InstanceID;
};

float3 DepthToWorldPosition(float depth, float2 projPosition)
{
    float4 projPos = float4(projPosition.x, projPosition.y, depth, 1.0f);
    float4 viewPos = mul(projPos, inverseProjection);
    float4 worldPos = mul(viewPos, inverseView);
    return worldPos.xyz / worldPos.w;
}
float2 ProjPositionToUV(float2 projPosition)
{
    projPosition.x += 1.0f;
    projPosition.y += 1.0f;
    projPosition.x *= 0.5f;
    projPosition.y *= 0.5f;
    projPosition.y = 1 - projPosition.y;
    return projPosition;
}

//Directional Light

VSOutput DirectionalLightVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    result.position = float4(input.position, 1.0f);
    result.InstanceID = InstanceID;
    result.projPosition = result.position;
    return result;
    
}

float4 DirectionalLightPixelFuction(VSOutput input) : SV_Target0
{
    input.projPosition /= input.projPosition.w;
    float2 uv = ProjPositionToUV(input.projPosition.xy);
    PixelResult pixelData = PixelDecode(
    InputGBuffer0.Sample(gsamPointWrap, uv),
    InputGBuffer1.Sample(gsamPointWrap, uv),
    InputGBuffer2.Sample(gsamPointWrap, uv),
    InputGBuffer3.Sample(gsamPointWrap, uv)
    );
    

    float depth = InputGBuffer4.Sample(gsamPointWrap, uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy);
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightData.Direction), normalize(-cameraDirection), 1.0f);
}


 // PointLight
float CalcAttenuation(float distance, float falloffStart, float falloffEnd)
{
    float d = pow(distance, 2);
    float r = pow(falloffEnd, 2);
    return pow(saturate(1 - pow(d / r, 2)), 2);
    //return pow(saturate((falloffEnd - distance) / (falloffEnd - falloffStart)), 2);
}

VSOutput PointLightVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    
    float3 worldPosition = input.position * lightInfo[InstanceID].FalloffEnd * 1.1f;
    worldPosition = worldPosition + lightInfo[InstanceID].Position;
    
    result.position = mul(float4(worldPosition, 1), viewProjection);
    result.projPosition = result.position;
    result.InstanceID = InstanceID;
    return result;
}

float4 PointLightPixelFuction(VSOutput input, bool isFrontFace : SV_IsFrontFace) : SV_Target0
{
    input.projPosition /= input.projPosition.w;
    float2 uv = ProjPositionToUV(input.projPosition.xy);
    PixelResult pixelData = PixelDecode(
    InputGBuffer0.Sample(gsamPointWrap, uv),
    InputGBuffer1.Sample(gsamPointWrap, uv),
    InputGBuffer2.Sample(gsamPointWrap, uv),
    InputGBuffer3.Sample(gsamPointWrap, uv)
    );
    float depth = InputGBuffer4.Sample(gsamPointWrap, uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy);
    
    //float3 cameraDirection = look;
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    float3 lightDirection = calcWorldPosition - lightData.Position;
    
    float distance = length(lightDirection);
    
    float atten = CalcAttenuation(distance, lightData.FalloffStart, lightData.FalloffEnd);
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightDirection), normalize(-cameraDirection), atten);
}


//Ambient Light

VSOutput AmbientLightVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    result.position = float4(input.position, 1.0f);
    result.InstanceID = InstanceID;
    result.projPosition = result.position;
    return result;
    
}

float4 AmbientLightPixelFuction(VSOutput input) : SV_Target0
{
    input.projPosition /= input.projPosition.w;
    float2 uv = ProjPositionToUV(input.projPosition.xy);
    PixelResult pixelData = PixelDecode(
    InputGBuffer0.Sample(gsamPointWrap, uv),
    InputGBuffer1.Sample(gsamPointWrap, uv),
    InputGBuffer2.Sample(gsamPointWrap, uv),
    InputGBuffer3.Load(input.position)
    );
    

    float depth = InputGBuffer4.Sample(gsamPointWrap, uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy);
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    
    return CustomAmbientLightCalculator(lightData, pixelData, normalize(lightData.Direction), normalize(cameraDirection), 1.0f);
}
