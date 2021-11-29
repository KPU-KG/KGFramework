#include "DXR_GLOBAL_DEFINE.hlsl"
#include "DXR_UTILL_LIGHT_CUSTOM_DIFFUSE.hlsl"

[numthreads(32, 32, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int2 launchIndex = dispatchThreadID;
    float2 size;
    output.GetDimensions(size.x, size.y);
    float2 ndc = ((((float2) launchIndex + 0.5f) / (float2) size) * 2.f - 1.f);
    ndc.y = -ndc.y;

    float4x4 inverseViewProj = mul(camera.inverseProjection, camera.inverseView);
    float4 unproj = mul(float4(ndc, 1, 1), inverseViewProj);
    float3 world = unproj.xyz / unproj.w;
    float depth = InputGBuffer4.Load(float3(launchIndex, 0)).x;
    float3 dir = normalize(world - camera.cameraWorldPosition);
    
    float4 color = float4(0, 0, 0, 1);
    if (depth == 1.0f)
    {
        color = GammaToLinear(shaderTextureCube[ambient.skyBoxId].SampleLevel(gsamAnisotoropicWrap, dir, 0));
    }
    else
    {
        world = DepthToWorldPosition(depth, ndc, inverseViewProj);
        Surface surface = PixelDecode(
            InputGBuffer0.Load(float3(launchIndex, 0)),
            InputGBuffer1.Load(float3(launchIndex, 0)),
            InputGBuffer2.Load(float3(launchIndex, 0)),
            InputGBuffer3.Load(float3(launchIndex, 0)));
        
        [unroll]
        for (int i = 0; i < 1; ++i)
        {
            //if (lightInfo[i].lightType.x == 0)
            {
                float4 shadow = rwTexture[lightInfo[i].lightType[1]][launchIndex].rrrr;
                shadow = clamp(shadow, float4(0.3, 0.3, 0.3, 1.0f), float4(1, 1, 1, 1));
                color += CustomLightCalculator(lightInfo[i], surface, normalize(lightInfo[i].Direction), normalize(-dir), 1.0) * shadow;
            }
        }
        color += CustomAmbientLightCalculator(lightInfo[0], surface, float3(0, 1, 0), dir, 1.0f, ambient.iblLut, ambient.iblIrrad, ambient.iblRad, 1, launchIndex);
    }
    output[launchIndex] = color;
}