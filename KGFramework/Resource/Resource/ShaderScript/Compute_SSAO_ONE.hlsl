#include "Define_Compute.hlsl"
#include "Compute_SSAO_DEFINE.hlsl"

[numthreads(1024, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 res = GetRes();
    uint3 CurPixel = uint3(dispatchThreadID.x % res.x, dispatchThreadID.x / res.x, 0);
    if(CurPixel.y < res.y)
    {
        float minDepth = 1.0f;
        float3 avgNormal = float3(0, 0, 0);
        uint3 FullResPixel = CurPixel * 2;
        
        [unroll]
        for (int i = 0; i < 2; i++)
        {
            [unroll]
            for (int j = 0; j < 2; j++)
            {
                float curDepth = InputGBuffer4.Load(FullResPixel, int2(j, i));
                minDepth = min(curDepth, minDepth);
                
                //float3 normalWorld = DecodeNormal(InputGBuffer2.Load(FullResPixel, int2(j, i)).xy);
                //avgNormal += mul(normalWorld, (float3x3)view);
            }
        }
        uint2 outputPosition = GetPositionBuffer(dispatchThreadID.x);
        float3 normalWorld = DecodeNormal(InputGBuffer2.Load(FullResPixel, int2(0, 0)).xy);
        //buffer0[dispatchThreadID.x].yzw = avgNormal * 0.25f;
        buffer0[outputPosition] = float4(ConvertZToLinearDepth(GetProjValue(projection), minDepth), mul(normalWorld, (float3x3) view));
    }
}