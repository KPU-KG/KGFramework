#ifndef __CAMERA_DEFINE__
#define __CAMERA_DEFINE__

cbuffer GSCameraData : register(b0)
{
    float4x4 view[6];
    float4x4 projection;
    
    float4x4 inverseView[6];
    float4x4 inverseProjection;
	
    float3 cameraWorldPosition;
    float1 pad0;
    float4 look[6];
};

#endif