#ifndef __CAMERA_DEFINE__
#define __CAMERA_DEFINE__

cbuffer GSCameraData : register(b0)
{
    float4x4 view[4];
    float4x4 projection[4];
    float4 cameraWorldPosition[4];
    float4 look;
};

#endif