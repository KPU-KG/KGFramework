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

TextureCube<float> shadowCube[] : register(t0, space1);


float PointShadowPCF(float3 toPixel, LightData lightData, ShadowData shadowData, float cosTheta)
{
    if (shadowData.shadowMapIndex[0] == 0)
    {
        return 1.0f;
    }
    float3 toPixelAbs = abs(toPixel);
    float z = max(toPixelAbs.x, max(toPixelAbs.y, toPixelAbs.z));
    float2 lightPerspectiveValue = float2(shadowData.shadowMatrix[0]._m22, shadowData.shadowMatrix[0]._m32);
    float depth = (lightPerspectiveValue.x * z + lightPerspectiveValue.y) / z;
    float bias = 0.005f * tan(acos(cosTheta));
    bias = clamp(bias, 0.0003f, 0.01f);
    return shadowCube[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamAnisotoropicCompClamp, normalize(toPixel), depth - bias);
}




float3 SphericialCoordToVec(float2 sphere)
{
    return float3(
    sin(sphere.x) * cos(sphere.y),
    sin(sphere.x) * sin(sphere.y),
    cos(sphere.x)
    );
}

float2 VectorToSphericialCoord(float3 vec)
{
    return float2(acos(vec.z / length(vec)), atan2(vec.y, vec.x));
}

float3 GetOffsetVector(float3 vec, uint index)
{
    static float2 offset[9] =
    {
        float2(-1,+1),
        float2(+0,+1),
        float2(+1,+1),
        float2(-1, +0),
        float2(+0,+0),
        float2(+1,+0),
        float2(-1, -1),
        float2(+0,-1),
        float2(+1,-1),
    };
    static float rad = 0.0174533f * 0.1f;
    float len = length(vec);
    return SphericialCoordToVec(VectorToSphericialCoord(vec) + offset[index] * rad);
}

float3 GetPoissonVector(float3 vec, float ratio, uint index)
{
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
    static float rad = 0.0174533f * 0.1f;
    float len = length(vec);
    return SphericialCoordToVec(VectorToSphericialCoord(vec) + poissonDisk[index] * rad * ratio);
}

float PointShadowPoissonPCF(float3 toPixel, LightData lightData, ShadowData shadowData, float cosTheta)
{
    if (shadowData.shadowMapIndex[0] == 0)
    {
        return 1.0f;
    }
    float3 toPixelAbs = abs(toPixel);
    float z = max(toPixelAbs.x, max(toPixelAbs.y, toPixelAbs.z));
    float2 lightPerspectiveValue = float2(shadowData.shadowMatrix[0]._m22, shadowData.shadowMatrix[0]._m32);
    float depth = (lightPerspectiveValue.x * z + lightPerspectiveValue.y) / z;
    float result = 0.0f;
    
    float bias = 0.005f * tan(acos(cosTheta));
    bias = clamp(bias, 0.0f, 0.01f);
    
    for (uint n = 0; n < 16; n++)
    {
        result += shadowCube[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamLinerCompClamp, GetPoissonVector(normalize(toPixel), 1.0f, n), depth - bias);
    }
    result /= 16.0f;
    return result;
}



float PointShadowPCSS(float3 toPixel, LightData lightData, ShadowData shadowData, float cosTheta)
{
    if (shadowData.shadowMapIndex[0] == 0)
    {
        return 1.0f;
    }
    float3 toPixelAbs = abs(toPixel);
    float z = max(toPixelAbs.x, max(toPixelAbs.y, toPixelAbs.z));
    float2 lightPerspectiveValue = float2(shadowData.shadowMatrix[0]._m22, shadowData.shadowMatrix[0]._m32);
    float depth = (lightPerspectiveValue.x * z + lightPerspectiveValue.y) / z;
    
    float bias = 0.005f * tan(acos(cosTheta));
    bias = clamp(bias, 0.0f, 0.01f);
    
    float avgBlockerDepth = 0.0f;
    float blockerCount = 0.0f;
    for (uint i = 0; i < 9; i += 1)
    {
        float3 toOffsetPixel = GetOffsetVector(normalize(toPixel), i);
        uint state = 0;
        float sampleDepth = shadowCube[shadowData.shadowMapIndex[0]].Sample(gsamPointClamp, toOffsetPixel);
        if (sampleDepth < depth - bias)
        {
            blockerCount += 1;
            avgBlockerDepth += sampleDepth;
        }
    }
    
    if (blockerCount == 0.0)
        return 1.0;
    else if (blockerCount >= 9)
        return 0.0;
    
    avgBlockerDepth /= blockerCount;
    
    float lightSize = 20.0f;
    float ratio = (abs(depth - avgBlockerDepth) * lightSize / avgBlockerDepth);
    
    float result = 0.0f;
    for (uint n = 0; n < 16; n++)
    {
        result += shadowCube[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamAnisotoropicCompClamp, GetPoissonVector(normalize(toPixel), 0.5f + pow(ratio * 32, 32), n),
        depth - bias);
    }
    result /= 16;
    return result;
}



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
    ShadowData shadowData = shadowInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy, mul(inverseProjection, inverseView));
    
    //float3 cameraDirection = look;
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    float3 lightDirection = calcWorldPosition - lightData.Position;
    
    float distance = length(lightDirection);
    
    float atten = CalcAttenuation(distance, lightData.FalloffStart, lightData.FalloffEnd);

    float shadowFactor = PointShadowPCSS(lightDirection, lightData, shadowData, dot(normalize(lightDirection), normalize(pixelData.wNormal)));
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightDirection), normalize(-cameraDirection), atten) * shadowFactor;
}

