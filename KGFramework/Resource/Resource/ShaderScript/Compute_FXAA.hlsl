#include "Define_Compute.hlsl"
  #define FXAA_PC 1
  #define FXAA_HLSL_5 1
  #define FXAA_QUALITY__PRESET 39
  #define FXAA_QUALITY__SUBPIX 1
#include "FXAA.hlsl"

float4 GetProjValue(float4x4 projMat)
{
    return float4(1.0f / projMat[0][0], 1.0f / projMat[1][1], projMat[3][2], -projMat[2][2]);
}

float ConvertZToLinearDepth(float4 projParams, float depth)
{
    float linearDepth = projParams.z / (depth + projParams.w);
    return linearDepth;
}

float3 DepthToWorldPosition(float depth, float2 projPosition, float4x4 inverseViewProj)
{
    float4 projPos = float4(projPosition.x, projPosition.y, depth, 1.0f);
    float4 worldPos = mul(projPos, inverseViewProj);
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

float2 UVToProjPosition(float2 projPosition)
{
    projPosition.x -= 1.0f;
    projPosition.y -= 1.0f;
    projPosition.x /= 0.5f;
    projPosition.y /= 0.5f;
    projPosition.y = 1 - projPosition.y;
    return projPosition;
}

[numthreads(32, 32, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    FxaaTex tex;
    tex.smpl = gsamAnisotoropicClamp;
    tex.tex = prevResult;
    float2 outputPosition = float2(float(dispatchThreadID.x + .5f) / texWidth, float(dispatchThreadID.y + .5f) / texHeight);
    float2 rcpro = rcp(float2(texWidth, texHeight));
    float3 result = FxaaPixelShader(outputPosition, 0, tex, tex, tex, rcpro, 0, 0, 0, 0.5, 0.125, 0.0833, 0, 0, 0, 0);
    outputResult[dispatchThreadID.xy] = float4(result, 1.0f);
}