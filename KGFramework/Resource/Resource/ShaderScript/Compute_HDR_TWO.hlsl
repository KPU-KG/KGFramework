#include "Define_Compute.hlsl"
#include "Compute_HDR_DEFINE.hlsl"
#define UNIT_COUNT 64

[numthreads(UNIT_COUNT, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadId : SV_DispatchThreadID)
{
    DownscaleInfo info;
    float width = 0;
    float height = 0;
    outputResult.GetDimensions(width, height);
    info.res = int2(width * 0.25f, height * 0.25f);
    info.domain = width * height * 0.0625f;
    info.groupSize = width * height * 1024 * 0.0625f;
    
   // ���� �޸𸮿� ID�� ����
    float favgLum = 0.f;

    if (dispatchThreadId.x < info.groupSize )
    {
        favgLum = prevBuffer0.Load(int3(dispatchThreadId.x, 0, 0)).x;
    }

    shaderCache[dispatchThreadId.x] = favgLum;

    GroupMemoryBarrierWithGroupSync(); // ����ȭ �� ���� ��������

    // 64���� 16���� �ٿ� ������
    if (dispatchThreadId.x % 4 == 0)
    {
        // �ֵ� �� �ջ�
        float fstepAvgLum = favgLum;

        fstepAvgLum += dispatchThreadId.x + 1 < info.groupSize ? shaderCache[dispatchThreadId.x + 1] : favgLum;

        fstepAvgLum += dispatchThreadId.x + 2 < info.groupSize ? shaderCache[dispatchThreadId.x + 2] : favgLum;

        fstepAvgLum += dispatchThreadId.x + 3 < info.groupSize ? shaderCache[dispatchThreadId.x + 3] : favgLum;

        // ��� �� ����
        favgLum = fstepAvgLum;

        shaderCache[dispatchThreadId.x] = fstepAvgLum;
    }

    GroupMemoryBarrierWithGroupSync(); // ����ȭ �� ���� ��������

    // 16���� 4�� �ٿ����
    if (dispatchThreadId.x % 16 == 0)
    {
        // �ֵ� �� �ջ�
        float fstepAvgLum = favgLum;

        fstepAvgLum += dispatchThreadId.x + 4 < info.groupSize ? shaderCache[dispatchThreadId.x + 4] : favgLum;

        fstepAvgLum += dispatchThreadId.x + 8 < info.groupSize ? shaderCache[dispatchThreadId.x + 8] : favgLum;

        fstepAvgLum += dispatchThreadId.x + 12 < info.groupSize ? shaderCache[dispatchThreadId.x + 12] : favgLum;

        // ��� �� ����
        favgLum = fstepAvgLum;
        shaderCache[dispatchThreadId.x] = fstepAvgLum;
    }
    
    GroupMemoryBarrierWithGroupSync(); // ����ȭ �� ���� ��������

    // 4���� 1�� �ٿ����
    if (dispatchThreadId.x == 0)
    {
        // �ֵ� �� �ջ�
        float fFinalLumValue = favgLum;

        fFinalLumValue += dispatchThreadId.x + 16 < info.groupSize ? shaderCache[dispatchThreadId.x + 16] : favgLum;

        fFinalLumValue += dispatchThreadId.x + 32 < info.groupSize ? shaderCache[dispatchThreadId.x + 32] : favgLum;

        fFinalLumValue += dispatchThreadId.x + 48 < info.groupSize ? shaderCache[dispatchThreadId.x + 48] : favgLum;

        fFinalLumValue /= 64.f;
        
        float lum = max(fFinalLumValue, 0.0001).xxxx;
        frameData[int2(0, 0)] = lerp(frameData[int2(0, 0)], lum, 0.05f);
    }
}