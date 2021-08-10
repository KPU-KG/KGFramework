#include "Define_Compute.hlsl"
#include "Compute_SSAO_DEFINE.hlsl"

#ifndef __BLUR_DEFINE__
#define __BLUR_DEFINE__

static const float sampleWeight[13] =
{
    0.002216,
    0.008764,
    0.026995,
    0.064759,
    0.120985,
    0.176033,
    0.199471,
    0.176033,
    0.120985,
    0.064759,
    0.026995,
    0.008764,
    0.002216
};

#define groupThreadCount 128
#define kernelHalf 6
//static const uint kernelHalf = 6;

static const uint groupThreads = groupThreadCount;
groupshared float4 sharedBlurInput[groupThreads];

void VerticalFilter(uint3 Gid, uint Gi, Texture2D<float4> input, RWTexture2D<float4> output)
{
    uint2 res = GetRes();
    int2 coord = int2(Gid.x, Gi - kernelHalf + (groupThreads - kernelHalf * 2) * Gid.y);
    coord = clamp(coord, int2(0, 0), int2(res.x - 1, res.y - 1));
    sharedBlurInput[Gi] = input.Load(int3(coord, 0));
    GroupMemoryBarrierWithGroupSync();
    
    if (Gi >= kernelHalf && Gi < (groupThreads - kernelHalf) && ((Gi - kernelHalf + (groupThreads - kernelHalf * 2) * Gid.y) < res.y))
    {
        float4 vOut = 0;
        [unroll]
        for (int i = -kernelHalf; i <= kernelHalf; ++i)
        {
            vOut += sharedBlurInput[Gi + i] * sampleWeight[i + kernelHalf];
        }
        output[coord] = float4(vOut.rgb, 1.0f);
    }
}

void HorizontalFilter(uint3 Gid, uint Gi, Texture2D<float4> input, RWTexture2D<float4> output)
{
    uint2 res = GetRes();
    int2 coord = int2(Gi - kernelHalf + (groupThreads - kernelHalf * 2) * Gid.x, Gid.y);
    coord = clamp(coord, int2(0, 0), int2(res.x - 1, res.y - 1));
    sharedBlurInput[Gi] = input.Load(int3(coord, 0));
    GroupMemoryBarrierWithGroupSync();
    
    if (Gi >= kernelHalf && Gi < (groupThreads - kernelHalf) && ((Gi - kernelHalf + (groupThreads - kernelHalf * 2) * Gid.x) < res.x))
    {
        float4 vOut = 0;
        [unroll]
        for (int i = -kernelHalf; i <= kernelHalf; ++i)
        {
            vOut += sharedBlurInput[Gi + i] * sampleWeight[i + kernelHalf];
        }
        output[coord] = float4(vOut.rgb, 1.0f);
    }
}
#endif