
cbuffer PassData : register(b1) 
{
	
}

struct InstanceData
{
    float4x4 world;
    uint materialIndex;
};
StructuredBuffer<InstanceData> objectInfo : register(t0);

Texture2D<float4> shaderTexture1[] : register(t0, space1);

TextureCube<float4> shaderTexture2[] : register(t0, space1);

TextureCube<float4> shaderTexture3[] : register(t0, space2);
