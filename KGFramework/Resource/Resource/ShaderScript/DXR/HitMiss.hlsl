#include "Define.hlsl"


[shader("closesthit")]
void Hit(inout Payload payload : SV_Payload, Built_in_attribute attr)
{
    Surface
    Surface surfece = UserSurfaceFunction();
    payload.color = 1.0f;
}

[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = 0.0f;
}