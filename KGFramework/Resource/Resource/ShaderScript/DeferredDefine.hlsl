#define MAX_COUNT_BONE 64


cbuffer PassData : register(b1) 
{
	
}

struct InstanceData
{
    float4x4 world;
    uint materialIndex;
    uint environmentMapIndex;
    uint2 padding;
};
StructuredBuffer<InstanceData> objectInfo : register(t0);

StructuredBuffer<float4x4> boneOffsetInfo : register(t0, space3);

//struct AnimationTransform
//{
//    float4x4 transform[MAX_COUNT_BONE];
//};

//StructuredBuffer<AnimationTransform> animationTransformInfo : register(t1, space3);

StructuredBuffer<float4x4> animationTransformInfo : register(t1, space3);


Texture2D<float4> shaderTexture[] : register(t0, space1);

TextureCube<float4> shaderTextureCube[] : register(t0, space1);

TextureCube<float4> shaderTexture3[] : register(t0, space2);

