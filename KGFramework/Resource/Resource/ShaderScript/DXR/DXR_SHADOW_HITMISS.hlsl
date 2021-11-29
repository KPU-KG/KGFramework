#include "DXR_GLOBAL_DEFINE_RAY.hlsl"
#include "DXR_DEFINE_LIGHT.hlsl"
#include "DXR_UTILL_LIGHT_CUSTOM.hlsl"

#ifndef __DXR_HLSL_SHADOW_HIT_MISS__
#define __DXR_HLSL_SHADOW_HIT_MISS__

[shader("closesthit")]
void Hit(inout Payload payload : SV_Payload, Built_in_attribute attr)
{
    payload.color = float4(0, 0, 0, 1);
    payload.recursionDepth = 30;
}

[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = float4(1, 1, 1, 1);
    payload.recursionDepth = 30;
}

#endif 

