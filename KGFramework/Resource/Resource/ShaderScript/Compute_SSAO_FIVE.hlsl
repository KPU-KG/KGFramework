#include "Define_Compute.hlsl"
#include "Compute_SSAO_DEFINE.hlsl"

[numthreads(32, 32, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID, uint gi : SV_GroupIndex)
{
    int texWidth = -1;
    int texHeight = -1;
    outputResult.GetDimensions(texWidth, texHeight);
    float2 uv = float2(float(dispatchThreadID.x + .5f) / texWidth, float(dispatchThreadID.y + .5f) / texHeight);
    float ao = saturate(prevBuffer0.SampleLevel(gsamLinearClamp, uv * 0.5f, 0).x);
    ao = lerp(ao, materialDatas[materialIndex].radius, materialDatas[materialIndex].offsetRadius);
    float3 buffer = InputGBuffer1.SampleLevel(gsamLinearClamp, uv, 0).xyz;
    outputResult[dispatchThreadID.xy] = float4(buffer, ao);
}