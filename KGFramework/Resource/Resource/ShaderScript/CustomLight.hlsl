#ifndef __CUSTOM_LIGHT_DEFINE__
#define __CUSTOM_LIGHT_DEFINE__

#include "GlobalDefine.hlsl"
#include "GBufferDefine.hlsl"
#include "LightDefine.hlsl"


float4 CustomLightCalculator(LightData light, PixelResult info, float3 lightDir, float3 cameraDir, float atten)
{
    // Diffuse Section
    float ndot1 = dot(info.wNormal, lightDir) * 0.5 + 0.5;
    ndot1 = pow(ndot1, 3);
    float3 diffuseColor = ndot1 * info.albedo * light.Strength * atten;
    
    // Rim Section
    float rim = dot(info.wNormal, cameraDir);
    float3 rimLight = pow(1 - rim, 8).xxx;
    rimLight = 0;
    
    // Specular Section
    float3 H = normalize(lightDir + cameraDir);
    float spec = saturate(dot(H, info.wNormal));
    spec = pow(spec, 64) * atten;
    
    float4 finalColor;
    finalColor.rgb = diffuseColor + spec + rimLight;
    finalColor.a = 1.0f;
    
    return finalColor;
}

#endif