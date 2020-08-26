#include "GlobalDefine.hlsl"
#include "LightDefine.hlsl"

struct VSOutput
{
    float4 position : SV_Position;
    float2 projPosition : POSITION;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

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

float3 DepthToWorldPosition(float depth, float2 projPosition)
{
    float4 projPos = float4(projPosition.x, projPosition.y, depth, 1.0f);
    float4 viewPos = mul(projPos, inverseProjection);
    float4 worldPos = mul(viewPos, inverseView);
    return worldPos.xyz /  worldPos.w;
}

VSOutput DirectionalLightVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    result.position = float4(input.position, 1.0f);
    result.InstanceID = InstanceID;
    
    result.uv = input.position.xy;
    result.projPosition = input.position.xy;
    result.uv += 1.0f;
    result.uv *= 0.5f;
    result.uv.y = 1 - result.uv.y;
    
    return result;
    
}

float4 DirectionalLightPixelFuction(VSOutput input) : SV_Target0
{
    PixelResult pixelData = PixelDecode(
    InputGBuffer0.Sample(gsamPointWrap, input.uv),
    InputGBuffer1.Sample(gsamPointWrap, input.uv),
    InputGBuffer2.Sample(gsamPointWrap, input.uv),
    InputGBuffer3.Sample(gsamPointWrap, input.uv)
    );
    float depth = InputGBuffer4.Sample(gsamPointWrap, input.uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition);
    
    
    return float4(DirectionalLightFunction(lightData, pixelData, calcWorldPosition), 1.0f);
}