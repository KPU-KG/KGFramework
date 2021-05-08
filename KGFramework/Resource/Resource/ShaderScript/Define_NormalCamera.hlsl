#ifndef __CAMERA_DEFINE__
#define __CAMERA_DEFINE__

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

#endif