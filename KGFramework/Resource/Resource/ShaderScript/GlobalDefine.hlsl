#ifndef __GLOBAL_DEFINE__
#define __GLOBAL_DEFINE__
struct VertexData
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
    float2 uv : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    uint4 bone : BONE;
    float4 weight : WEIGHT;
};

cbuffer CameraData : register(b0)
{
	float4x4 view;
	float4x4 projection;
	float4x4 viewProjection;
	
    float4x4 inverseView;
    float4x4 inverseProjection;
    float4x4 inverseViewProjection;
	
	float3 cameraWorldPosition;
    float1 pad0;
	float3 look;
    float1 pad1;
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotoropicWrap : register(s4);
SamplerState gsamAnisotoropicClamp : register(s5);
#endif
