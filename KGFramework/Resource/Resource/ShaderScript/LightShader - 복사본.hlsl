#include "GlobalDefine.hlsl"
#include "LightDefine.hlsl"

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

float3 DirectionalLightFunction(LightData lightData, PixelResult pixelData, float3 worldPosition)
{
    if (length(pixelData.wNormal) > 1.5f)
    {
        discard;
    }
    
    float3 vToLight = -lightData.Direction;
    float fDiffuseFactor = max(dot(vToLight, pixelData.wNormal), 0.0f);
    float fSpecularFactor = 0.0f;
    
    float3 vNormal = normalize(pixelData.wNormal);
    float3 vToCamera = normalize(cameraWorldPosition - worldPosition);
    
    if (fDiffuseFactor > 0.0f)
    {
        //if (pixelData.specular != 0.0f)
        {
            float specular = 3.1f;
            float3 vHalf = normalize(vToCamera + vToLight);
            fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), specular);
        }
    }
    return (pixelData.albedo * (lightData.Strength * fDiffuseFactor)) + (lightData.Strength * fSpecularFactor);
}

VSOutput DirectionalLightVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    result.position = float4(input.position, 1.0f);
    result.InstanceID = InstanceID;
    result.projPosition = result.position;
    //result.projPosition = input.position.xy;
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
    
    return float4(DirectionalLightFunction(lightData, pixelData, calcWorldPosition), 1.0f);
}


 // PointLight

float CalcAttenuation(float distance, float falloffStart, float falloffEnd)
{
    return saturate((falloffEnd - distance) / (falloffEnd - falloffStart));
}

float3 PointLightFunction(LightData lightData, PixelResult pixelData, float3 worldPosition)
{
    if (length(pixelData.wNormal) > 1.5f)
    {
        return (float4(0.0f, 0.0f, 0.0f, 0.0f));
        discard;
    }
    //return (float4(0.01f, 0.0f, 0.0f, 0.0f));
    
    
    
    float3 vToLight = lightData.Position - worldPosition;
    float fDistance = length(vToLight);
    if (fDistance <= lightData.FalloffEnd * 1.1f)
    {

        float fSpecularFactor = 0.0f;
        
        float3 vNormal = normalize(pixelData.wNormal);
        float3 vToCamera = normalize(cameraWorldPosition - worldPosition);
        
        vToLight /= fDistance;
        float fDiffuseFactor = max(dot(vToLight, vNormal), 0.0f);
        
        if (fDiffuseFactor > 0.0f)
        {
            //if (pixelData.specular != 0.0f)
            {
                float specular = 3.1f;
                float3 vHalf = normalize(vToCamera + vToLight);
                fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), specular);
            }
        }
    
        float fAttenuationFactor = CalcAttenuation(fDistance, lightData.FalloffStart, lightData.FalloffEnd);
        return (pixelData.albedo * (lightData.Strength * fDiffuseFactor)) + (lightData.Strength * fSpecularFactor) * fAttenuationFactor;
    }
    return (float4(0.0f, 0.0f, 0.0f, 0.0f));
}

VSOutput PointLightVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    float3 worldPosition = input.position * lightInfo[InstanceID].FalloffEnd * 2.0f;
    //float3 worldPosition = input.position;
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
    
    
    //return float4(0, 0, 1, 1);
    
    return float4(PointLightFunction(lightData, pixelData, calcWorldPosition), 1.0f);
}


