#include "DXR_GLOBAL_DEFINE_RAY.hlsl"
#include "DXR_DEFINE_LIGHT.hlsl"
#include "DXR_UTILL_LIGHT_CUSTOM.hlsl"

[shader("raygeneration")]
void RayGeneration()
{
    int2 launchIndex = DispatchRaysIndex().xy;
    int2 dimensions = DispatchRaysDimensions().xy;
    float2 ndc = ((((float2) launchIndex + 0.5f) / (float2) dimensions) * 2.f - 1.f);
    ndc.y = -ndc.y;

    float4x4 inverseViewProj = mul(camera.inverseProjection, camera.inverseView);
    float4 unproj = mul(float4(ndc, 0, 1), inverseViewProj);
    float3 world = unproj.xyz / unproj.w;
    float depth = InputGBuffer4.Load(float3(launchIndex, 0)).x;
    float3 dir = normalize(world - camera.cameraWorldPosition);
    
    //output[launchIndex] = TraceRadiance(camera.cameraWorldPosition, normalize(world - camera.cameraWorldPosition), 0);
    
    float4 color = output[launchIndex];
    world = DepthToWorldPosition(depth, ndc, inverseViewProj);
    
    Surface surface = PixelDecode(
        InputGBuffer0.Load(float3(launchIndex, 0)),
        InputGBuffer1.Load(float3(launchIndex, 0)),
        InputGBuffer2.Load(float3(launchIndex, 0)),
        InputGBuffer3.Load(float3(launchIndex, 0))
    );
    
    float3 reflec = reflect(dir, surface.wNormal);
    rwTexture[ambient.specularOutput][launchIndex] = TraceRadiance(world, normalize(reflec), 1, depth != 1.0f && surface.roughness < 0.6f);
    //rwTexture[ambient.specularOutput][launchIndex] = TraceRadiance(world, normalize(reflec), 1, depth != 1.0f);
}