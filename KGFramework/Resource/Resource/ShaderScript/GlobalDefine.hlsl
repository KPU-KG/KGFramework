struct VertexData
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

cbuffer CameraData : register(b0)
{
	float4x4 view;
	float4x4 projection;
	float4x4 viewProjection;
	float3 cameraWorldPosition;
	float3 look;
};
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

Texture2D<float4> shaderTexture2[] : register(t0, space2);
