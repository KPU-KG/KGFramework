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
    
    float4 color = float4(0, 0, 0, 1);
    world = DepthToWorldPosition(depth, ndc, inverseViewProj);
    
    Surface surface = PixelDecode(
        InputGBuffer0.Load(float3(launchIndex, 0)),
        InputGBuffer1.Load(float3(launchIndex, 0)),
        InputGBuffer2.Load(float3(launchIndex, 0)),
        InputGBuffer3.Load(float3(launchIndex, 0))
    );
    world += surface.wNormal * 0.1f;
    //color = float4(surface.albedo, 1);
    // Directional Light
    [unroll]
    for (int i = 0; i < 1; ++i)
    {
        //if (lightInfo[i].lightType.x == 0)
        {
            float4 shadow = float4(1, 1, 1, 1);
            shadow = TraceShadow(world, normalize(-lightInfo[i].Direction), 0, depth != 1.0f);
            //shadow = clamp(shadow, float4(0.3, 0.3, 0.3, 1.0f), float4(1, 1, 1, 1));
            rwTexture[lightInfo[0].lightType[1]][launchIndex] = shadow;
            output[launchIndex] = shadow;
            //color = CustomLightCalculator(lightInfo[i], surface, normalize(lightInfo[i].Direction), normalize(-dir), 1.0) * shadow;
        }
    }
}