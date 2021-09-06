#ifndef __DXR_HLSL_DEFINE__
#define __DXR_HLSL_DEFINE__

RWTexture2D<float4> colorOutput : register(u0);

RaytracingAccelerationStructure sceneAs : register(t0);

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
    double gameTime;
};

struct Payload
{
    float4 color;
};

struct Built_in_attribute
{
    float BaryX;
    float BaryY;
};

#endif