#include "Define_Compute.hlsl"
#define UNIT_COUNT 1024

groupshared float shaderCache[UNIT_COUNT];
static const float4 lumFactor = float4(0.299, 0.587, 0.114, 0);

struct DownscaleInfo
{
    uint2 res;
    uint domain;
    uint groupSize;
};

float DownScale4x4(uint2 CurPixel, uint groupThreadId, DownscaleInfo info)
{
    float avgLum = 0.f;

    if (CurPixel.y < info.res.y)
    {
        int3 iFullResPos = int3(CurPixel * 4, 0);
        float4 vDownScaled = float4(0.f, 0.f, 0.f, 0.f);

        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            [unroll]
            for (int j = 0; j < 4; ++j)
            {
                vDownScaled += clamp(prevResult.Load(iFullResPos + int3(j, i, 0)), 0, 3);
            }
        }
        vDownScaled /= 16;
        
        // �ȼ��� �ֵ� �� ���
        avgLum = dot(vDownScaled, lumFactor);

        // ���� �޸𸮿� ��� ���
        shaderCache[groupThreadId] = avgLum;
    }

    // ����ȭ �� ���� �ܰ��
    GroupMemoryBarrierWithGroupSync();

    return avgLum;
}

float DownScale1024to4(uint dispachThreadId, uint groupThreadId, float avgLum, DownscaleInfo info)
{
    // �ٿ���� �ڵ带 Ȯ��
    for (uint iGroupSize = 4, iStep1 = 1, iStep2 = 2, iStep3 = 3;
        iGroupSize < 1024;
        iGroupSize *= 4, iStep1 *= 4, iStep2 *= 4, iStep3 *= 4)
    {
        if (groupThreadId % iGroupSize == 0)
        {
            float fStepAvgLum = avgLum;

            fStepAvgLum += dispachThreadId + iStep1 < info.domain ? shaderCache[groupThreadId + iStep1] : avgLum;

            fStepAvgLum += dispachThreadId + iStep2 < info.domain ? shaderCache[groupThreadId + iStep2] : avgLum;

            fStepAvgLum += dispachThreadId + iStep3 < info.domain ? shaderCache[groupThreadId + iStep3] : avgLum;

            // ��� �� ����
            avgLum = fStepAvgLum;
            shaderCache[groupThreadId] = fStepAvgLum;
        }
        // ����ȭ �� ��������
        GroupMemoryBarrierWithGroupSync();
    }
    return avgLum;
}

void DownScale4to1(uint dispatchThreadId, uint groupThreadId, uint groupId, float avgLum, DownscaleInfo info)
{
    if (groupThreadId == 0)
    {
        //  ������ �׷쿡 ���� ��� �ֵ� �� ���
        float fFinalAvgLum = avgLum;

        fFinalAvgLum += dispatchThreadId + 256 < info.domain ? shaderCache[groupThreadId + 256] : avgLum;

        fFinalAvgLum += dispatchThreadId + 512 < info.domain ? shaderCache[groupThreadId + 512] : avgLum;

        fFinalAvgLum += dispatchThreadId + 768 < info.domain ? shaderCache[groupThreadId + 768] : avgLum;

        fFinalAvgLum /= 1024.f;

        // ���� ���� ID UAV�� ��� �� ���� ��������
        buffer0[int2(groupId, 0)] = fFinalAvgLum.xxxx;
    }
}