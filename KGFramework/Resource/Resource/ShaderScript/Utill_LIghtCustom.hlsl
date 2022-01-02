#ifndef __CUSTOM_LIGHT_DEFINE__
#define __CUSTOM_LIGHT_DEFINE__

#include "Define_Global.hlsl"
#include "Define_GBuffer.hlsl"
#include "Define_Light.hlsl"

#define PI  3.14159265359

float CalcSpotFactor(float3 vToLight, LightData light)
{
    float cosPhi = cos(light.Phi / 2.0f);
    float cosTheta = cos(light.Theta / 2.0f);
    float fAlpha = max(dot(vToLight, light.Direction), 0.0f);
    float fSpotFactor = pow(max(((fAlpha - cosPhi) / (cosTheta - cosPhi)), 0.0f), light.FalloffEnd);
    return fSpotFactor;
}

float CalcAttenuation(float distance, float falloffStart, float falloffEnd)
{
    float d = pow(distance, 2);
    float r = pow(falloffEnd, 2);
    return pow(saturate(1 - pow(d / r, 2)), 2);
}

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

float4 CustomLightCalculator(LightData light, Surface info, float3 lightDir, float3 cameraDir, float atten)
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
    float D = ndfGGX(NDotH, clamp(info.roughness, 0.0001, 1));
    float G = gaSchlickGGX(NDotL, NDotV, clamp(info.roughness, 0.0001, 1));
 
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0.0f, 0.0f, 0.0f), info.metalic.xxx);
 
    // Lambert diffuse BRDF.
    float3 diffuseBRDF = NDotL * (info.albedo - info.albedo * info.metalic);
    //float3 diffuseBRDF = kd * info.albedo / 3.141592;
    
    // Cook-Torrance specular microfacet BRDF.
    float3 specularColor = lerp(0.08 * info.specular.xxx, info.albedo, info.metalic.xxx);
    float3 specularBRDF = (F * D * G) / max(0.00001f, 4.0f * NDotL * NDotV) * specularColor;

    return float4((diffuseBRDF + specularBRDF) * light.Strength * atten, 1.0f);
    
    
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

uint querySpecularTextureLevels(uint irrad)
{
    uint width, height, levels;
    shaderTextureCube[irrad].GetDimensions(0, width, height, levels);
    return levels;
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float3 a = float3((1.0f - roughness).xxx);
    return F0 + (max(a, F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float4 CustomAmbientLightCalculator(LightData light, Surface info, float3 lightDir, float3 cameraDir, float atten, uint lutIndex, uint diffuseRad, uint specularRad)
{
    
    float3 L = -lightDir;
    float3 V = cameraDir;
    float3 N = info.wNormal;
    float3 H = normalize(L + V);
    
    float NDotV = saturate(dot(N, V));
    float NDotL = saturate(dot(N, L));
    float NDotH = saturate(dot(N, H));
    float VDotH = saturate(dot(V, H));
    
    float cosLo = max(dot(N, -V), 0.0f);
    
    float Fdielectric = 0.04f;
    float3 F0 = lerp(Fdielectric.xxx, info.albedo.xyz, info.metalic.xxx);
    
    float3 F = fresnelSchlickRoughness(NDotV, F0, 1 - info.roughness);
    float D = ndfGGX(NDotH, clamp(1 - info.roughness, 0.0001, 1));
    float G = gaSchlickGGX(NDotL, NDotV, clamp(1 - info.roughness, 0.0001, 1));
 
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0.0f, 0.0f, 0.0f), info.metalic.xxx);
    //float3 reflec = reflect(-cameraDir, info.wNormal);
    float3 reflec = reflect(cameraDir, info.wNormal);
    
    float3 diffuseIrradiance = GammaToLinear(shaderTextureCube[diffuseRad].Sample(gsamAnisotoropicWrap, N).rgb);
    float3 diffuseIBL = kd * info.albedo * diffuseIrradiance;
    
    float2 specularBRDF = shaderTexture[lutIndex].Sample(gsamLinearClamp, float2(NDotV, (1 - info.roughness))).rg;
    uint specularTextureLevel = querySpecularTextureLevels(specularRad);
    float3 specularIrradiance = GammaToLinear(shaderTextureCube[specularRad].SampleLevel(gsamAnisotoropicWrap, normalize(reflec), specularTextureLevel * (info.roughness)).rgb);
    float3 specularIBL = (F * specularBRDF.x + specularBRDF.y) * specularIrradiance;
    return float4(atten * (diffuseIBL + specularIBL), 1.0f);
}

#endif