#include "Define_Compute.hlsl"
#include "Compute_HDR_DEFINE.hlsl"

[numthreads(UNIT_COUNT, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadId : SV_GroupThreadID, int3 dispatchThreadId : SV_DispatchThreadID)
{
    
    DownscaleInfo info;
    float width = 0;
    float height = 0;
    outputResult.GetDimensions(width, height);
    info.res = int2(width * 0.25f, height * 0.25f);
    info.domain = width * height * 0.0625f;
    info.groupSize = width * height * 1024 * 0.0625f;
    
    uint2 vCurPixel = uint2(dispatchThreadId.x % info.res.x, dispatchThreadId.x / info.res.x);

    // 16 픽셀 그룹을 하나의 픽셀로 줄여 공유 메모리에 저장
    float favgLum = DownScale4x4(vCurPixel, groupThreadId.x, info);

    // 1024에서 4로 다운스케일
    favgLum = DownScale1024to4(dispatchThreadId.x, groupThreadId.x, favgLum, info);

    // 4에서 1로 다운스케일
    DownScale4to1(dispatchThreadId.x, groupThreadId.x, groupId.x, favgLum, info);
}