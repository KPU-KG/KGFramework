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
    return (abs(position.x) < 1.0f) && (abs(position.y) < 1.0f) && (abs(position.z) < 1.0f);
}

float DirectionalShadowCascadePCF(float3 worldPosition, LightData lightData, ShadowData shadowData, out uint id)
{
    if (shadowData.shadowMapIndex[0] == 0)
    {
        id = 0;
        return 1.0f;
    }
    
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
    static float2 poissonDisk[16] = 
        {
        float2(-0.94201624, -0.39906216),
        float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870),
        float2(0.34495938, 0.29387760),
        
        float2(-0.91588581, 0.45771432),
        float2(-0.81544232, -0.87912464),
        float2(-0.38277543, 0.27676845),
        float2(0.97484398, 0.75648379),
        
        float2(0.44323325, -0.97511554),
        float2(0.53742981, -0.47373420),
        float2(-0.26496911, -0.41893023),
        float2(0.79197514, 0.19090188),
        
        float2(-0.24188840, 0.99706507),
        float2(-0.81409955, 0.91437590),
        float2(0.19984126, 0.78641367),
        float2(0.14383161, -0.14100790)
    };
    float result = 0.0f;
    for (uint n = 0; n < 8; n++)
    {
        result += shadowArray[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamAnisotoropicCompClamp, float3(uv + (poissonDisk[n] / 1400.0f), index), (depth) - 0.001f);
    }
    result /= 8.0f;
    return result;
    return shadowArray[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamLinerCompClamp, float3(uv, index), (depth) - 0.001f);
}


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

