#ifndef __CUSTOM_LIGHT_DEFINE__
#define __CUSTOM_LIGHT_DEFINE__

#include "GlobalDefine.hlsl"
#include "GBufferDefine.hlsl"
#include "DeferredDefine.hlsl"
#include "LightDefine.hlsl"

#define PI  3.14159265359


// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float4 CustomLightCalculator(LightData light, PixelResult info, float3 lightDir, float3 cameraDir, float atten)
{
    float3 L = -lightDir;
    float3 V = cameraDir;
    float3 N = info.wNormal;
    float3 H = normalize(L + V);
    
    float NDotV = saturate(dot(N, V));
    float NDotL = saturate(dot(N, L));
    float NDotH = saturate(dot(N, H));
    float VDotH = saturate(dot(V, H));
    
    float Fdielectric = 0.04f;
    float3 F0 = lerp(Fdielectric.xxx, info.albedo.xyz, info.metalic.xxx);
 
 
    float3 F = fresnelSchlick(F0, VDotH);
    float D = ndfGGX(NDotH, info.roughness);
    float G = gaSchlickGGX(NDotL, NDotV, info.roughness);
 
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0.0f, 0.0f, 0.0f), info.metalic.xxx);
 
    // Lambert diffuse BRDF.
    float3 diffuseBRDF = kd * info.albedo;
    
    // Cook-Torrance specular microfacet BRDF.
    float3 specularBRDF = (F * D * G) / max(0.00001f, 4.0f * NDotL * NDotV);    
    
    return float4((diffuseBRDF + specularBRDF) * NDotL * light.Strength * atten, 1.0f);
    
    
    //// Diffuse Section
    //float ndot1 = dot(info.wNormal, lightDir) * 0.5 + 0.5;
    //ndot1 = pow(ndot1, 3);
    //float3 diffuseColor = ndot1 * info.albedo * light.Strength * atten;
    
    //// Rim Section
    //float rim = dot(info.wNormal, cameraDir);
    //float3 rimLight = pow(1 - rim, 8).xxx;
    ////rimLight = 0;
    
    //// Specular Section
    //float3 H = normalize(lightDir + cameraDir);
    //float spec = saturate(dot(H, info.wNormal));
    //spec = pow(spec, 64) * atten;
    //float3 specColor = lerp(0.08 * info.specular.xxx, info.albedo, info.metailic) * spec;
    
    //float4 finalColor;
    //finalColor.rgb = diffuseColor + specColor;
    //finalColor.a = 1.0f;
    
    //return finalColor;
}

float4 CustomAmbientLightCalculator(LightData light, PixelResult info, float3 lightDir, float3 cameraDir, float atten)
{
    
    float3 L = -lightDir;
    float3 V = cameraDir;
    float3 N = info.wNormal;
    float3 H = normalize(L + V);
    
    float NDotV = saturate(dot(N, V));
    float NDotL = saturate(dot(N, L));
    float NDotH = saturate(dot(N, H));
    float VDotH = saturate(dot(V, H));
    
    float Fdielectric = 0.04f;
    float3 F0 = lerp(Fdielectric.xxx, info.albedo.xyz, info.metalic.xxx);
    
    float3 F = fresnelSchlick(F0, VDotH);
    float G = gaSchlickGGX(NDotL, NDotV, info.roughness);
 
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0.0f, 0.0f, 0.0f), 1.0f.xxx - info.metalic.xxx);
    float3 reflec = reflect(-cameraDir, info.wNormal);
    return float4(kd * shaderTextureCube[16].Sample(gsamAnisotoropicWrap, reflec).rgb * info.albedo, 1.0f);
}

#endif