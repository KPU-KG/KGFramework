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

bool isInPosition(float3 position)
{
    return (abs(position.x) <= 1.0f) && (abs(position.y) <= 1.0f) && (abs(position.z) <= 1.0f);
}

float DirectionalShadowCascadePCF(float3 worldPosition, LightData lightData, ShadowData shadowData, out uint id)
{
    float2 uv = float2(1.0f, 1.0f);
    float depth = 1.0f;
    uint index = 0;
    
    {
        float4 projPos = mul(float4(worldPosition, 1.0f), shadowData.shadowMatrix[0]);
        float3 projPos3 = projPos.xyz / projPos.w;
        uv = ProjPositionToUV(projPos3.xy);
        depth = projPos3.z;
        index = 0;

    }
    
    for (uint cascade = 0; cascade < 3; cascade++)
    {
        float4 projPos = mul(float4(worldPosition, 1.0f), shadowData.shadowMatrix[cascade + 1]);
        float3 projPos3 = projPos.xyz / projPos.w;
        if (isInPosition(projPos3))
        {
            uv = ProjPositionToUV(projPos3.xy);
            depth = projPos3.z;
            index = cascade + 1;
            break;
        }
    }
    id = index;
    return shadowArray[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamLinerCompClamp, float3(uv, index), (depth) - 0.001f);
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
    
    uint id;
    
    float4 cascadeDebugColor[4] = 
        {
        float4(1.0f,1.0f,1.0f, 1.0f),
        float4(1.0f,0.0f,0.0f, 1.0f),
        float4(0.0f,1.0f,0.0f, 1.0f),
        float4(0.0f,0.0f,1.0f, 1.0f),
    };
    
    float shadowFactor = DirectionalShadowCascadePCF(calcWorldPosition, lightData, shadowData, id);
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightData.Direction), normalize(-cameraDirection), 1.0f) * shadowFactor;
    //return CustomLightCalculator(lightData, pixelData, normalize(lightData.Direction), normalize(-cameraDirection), 1.0f) * shadowFactor * cascadeDebugColor[id];
}

