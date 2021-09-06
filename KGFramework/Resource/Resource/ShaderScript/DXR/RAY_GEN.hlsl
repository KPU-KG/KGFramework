#include "Define.hlsl"

static const float FLT_MAX = asfloat(0x7F7FFFFF);

[shader("raygeneration")]
void RayGeneration()
{
    int2 launchIndex = DispatchRaysIndex().xy;
    int2 dimensions = DispatchRaysDimensions().xy;
    
    float2 ndc = ((((float2) launchIndex + 0.5f) / (float2) dimensions) * 2.f - 1.f);
    ndc.y = -ndc.y;
    
    RayDesc ray;
    ray.Origin = cameraWorldPosition;
    float4 unproj = mul(float4(ndc, 0, 1), mul(inverseProjection, inverseView));
    float3 world = unproj.xyz / unproj.w;
    ray.Direction = normalize(world - cameraWorldPosition);
    ray.TMin = 0.0f;
    ray.TMax = FLT_MAX;
    
    Payload payload;
    TraceRay(
        sceneAs,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        0xFF,
        0,
        1,
        0,
        ray,
        payload
    );
    
    colorOutput[launchIndex] = payload.color;
}