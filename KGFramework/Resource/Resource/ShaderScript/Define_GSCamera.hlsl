#ifndef __CAMERA_DEFINE__
#define __CAMERA_DEFINE__

cbuffer GSCameraData : register(b0)
{
    float4x4 view[6];
    float4x4 projection[6];
    float4x4 viewProjection[6];
	
    float4x4 inverseView[6];
    float4x4 inverseProjection[6];
    float4x4 inverseViewProjection[6];
	
    float3 cameraWorldPosition;
    float1 pad0;
    float4 look[6];
};

#endif