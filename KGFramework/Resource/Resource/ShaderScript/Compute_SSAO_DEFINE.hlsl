#include "Define_Compute.hlsl"

float4 GetProjValue(float4x4 projMat)
{
    return float4(1 / projMat[0][0], 1 / projMat[1][1], projMat[3][2], projMat[2][2]);
}

float ConvertZToLinearDepth(float4 projParams,float depth)
{
    float linearDepth = projParams.z / (depth + projParams.w);
    return linearDepth;
}

uint2 GetRes()
{
    uint width = 0;
    uint height = 0;
    outputResult.GetDimensions(width, height);
    return uint2(width / 4, height / 4);
}

float2 GetResRcp()
{
    uint2 res = GetRes();
    return float2(1.0f / res.x, 1.0f / res.y);
}

static const float offsetRadius = 0.1f;
static const float radius = 0.05f;
static const float numSamplesRcp = 1.0f / 8.0f;
static const uint numSample = 8;

static const float2 SampleOffsets[numSample] =
{
    float2(+0.2803166f, +0.08997212f),
    float2(-0.5130632f, +0.6877457f),
    float2(+0.425495f, +0.8665376f),
    float2(+0.8732584f, +0.3858971f),
    float2(+0.0498111f, -0.6287371f),
    float2(-0.9674183f, +0.1236534f),
    float2(-0.3788098f, -0.09177673f),
    float2(+0.6985874f, -0.5610316f)
};

float ComputeAO(int2 centerPixelPos, float2 centerClipPos, float4 projParams)
{
    float centerDepth = InputGBuffer4.Load(int3(centerPixelPos.xy, 0));
    
    float3 centerPos = float3(0, 0, 0);
    centerPos.xy = centerClipPos * projParams.xy * centerDepth;
    centerPos.z = centerDepth;
    
    float3 centerNormal = DecodeNormal(InputGBuffer2.Load(int3(centerPixelPos.xy, 1)).xy);
    
    float rotationAngle = dot(float2(centerClipPos), float2(73.0, 197.0));
    float2 randSinCos = float2(0, 0);
    sincos(rotationAngle, randSinCos.x, randSinCos.y);
    float2x2 randRotMat = float2x2(randSinCos.y, -randSinCos.x, randSinCos.x, randSinCos.y);
    
    float ao = 0.0f;
    [unroll]
    for (uint i = 0; i < numSample; i++)
    {
        float2 sampleOff = offsetRadius.xx * mul(SampleOffsets[i], randRotMat);
        float curDepth = InputGBuffer4.Load(int3(centerPixelPos + int2(sampleOff.x, -sampleOff.y), 0));
        
        float3 curPos;
        curPos.xy = (centerClipPos + 2.0 * sampleOff * GetResRcp()) * projParams.xy * curDepth;
        curPos.z = curDepth;
        float3 centerToCurPos = curPos - centerPos;
        float lenCenterToCurPos = length(centerToCurPos);
        float angleFactor = 1.0f - dot(centerToCurPos / lenCenterToCurPos, centerNormal);
        float distFactor = lenCenterToCurPos / radius;
        ao += saturate(max(distFactor, angleFactor));
    }
    return ao * numSamplesRcp;
}

uint2 GetPositionBuffer(uint x)
{
    uint width;
    uint height;
    buffer0.GetDimensions(width, height);
    return uint2(x % width, x / height);
}