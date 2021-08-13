#include "Define_Global.hlsl"

StructuredBuffer<float4x4> boneOffsetInfo : register(t0, space3);
StructuredBuffer<float4x4> animationTransformInfo : register(t1, space3);

#define MAX_COUNT_BONE 64

float4x4 GetWorldMatrix(uint instanceID, VertexData vertex)
{
    float4x4 animationMatrix = (float4x4) 0.0f;
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        int animationIndex = (MAX_COUNT_BONE * instanceID) + vertex.bone[i];
        float4x4 bone = boneOffsetInfo[vertex.bone[i]];
        float4x4 anim = animationTransformInfo[animationIndex];
        float4x4 vTobone = mul(bone, anim);
        animationMatrix += vertex.weight[i] * vTobone;
    }
    return animationMatrix;
}