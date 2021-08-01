#include "Define_Compute.hlsl"
  #define FXAA_PC 1
  #define FXAA_HLSL_5 1
  #define FXAA_QUALITY__PRESET 15
#include "FXAA.hlsl"

[numthreads(32, 32, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int texWidth = -1;
    int texHeight = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    FxaaTex tex;
    tex.smpl = gsamLinearClamp;
    tex.tex = prevResult;
    float2 outputPosition = float2(float(dispatchThreadID.x + .5f) / texWidth, float(dispatchThreadID.y + .5f) / texHeight);
    float2 rcpro = rcp(float2(texWidth, texHeight));
    float3 result = FxaaPixelShader(outputPosition, 0, tex, tex, tex, rcpro, 0, 0, 0, 1.0, 0.166, 0.0312, 0, 0, 0, 0);
    //float3 result = prevResult.SampleLevel(gsamPointClamp, outputPosition, 0);
    outputResult[dispatchThreadID.xy] = float4(result, 1.0f);
}