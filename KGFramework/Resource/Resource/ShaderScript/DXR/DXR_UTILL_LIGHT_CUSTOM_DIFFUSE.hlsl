#ifndef __CUSTOM_LIGHT_DEFINE__
#define __CUSTOM_LIGHT_DEFINE__

#include "DXR_GLOBAL_DEFINE.hlsl"
#include "DXR_DEFINE_LIGHT.hlsl"

#define PI  3.14159265359

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

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

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float3 a = float3((1.0 - roughness).xxx);
    return F0 + (max(a, F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 EnvBRDFApprox(float3 SpecularColor, float Roughness, float NoV)
{

    const float4 c0 = { -1, -0.0275, -0.572, 0.022 };

    const float4 c1 = { 1, 0.0425, 1.04, -0.04 };

    float4 r = Roughness * c0 + c1;

    float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;

    float2 AB = half2(-1.04, 1.04) * a004 + r.zw;

    return SpecularColor * AB.x + AB.y;

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
 
 
    float NDF = DistributionGGX(N, H, info.roughness);
    float G = GeometrySmith(N, V, L, info.roughness);
    float3 F = fresnelSchlick(VDotH, F0);

    float3 numerator = NDF * G * F;

    float denominator = 4 * NDotV * NDotL + 0.0001f;
    float3 specular = numerator / denominator * info.specular;

    float3 kS = F;
    float3 kD = float3(1, 1, 1) - kS;
    kD *= 1.0f - info.metalic;

 
    return float4((kD * info.albedo / PI + specular) * light.Strength * atten * NDotL, 1.0f);
   
}

uint querySpecularTextureLevels(uint irrad)
{
    uint width, height, levels;
    shaderTextureCube[irrad].GetDimensions(0, width, height, levels);
    return levels;
    
}

float4 CustomAmbientLightCalculator(LightData light, Surface info, float3 lightDir, float3 cameraDir, float atten, uint lutIndex, uint diffuseRad, uint specularRad, uint recursionDepth, uint2 uv)
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
    
    float3 F = fresnelSchlickRoughness(NDotV, F0, info.roughness);
 
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - info.metalic;
    float3 reflec = reflect(-cameraDir, info.wNormal);
    //float3 reflec = reflect(cameraDir, info.wNormal);
    
    float3 diffuseIrradiance = GammaToLinear(shaderTextureCube[diffuseRad].SampleLevel(gsamAnisotoropicWrap, -N, 0).rgb) * info.albedo;
    
    float2 specularBRDF = shaderTexture[lutIndex].SampleLevel(gsamLinearClamp, float2(NDotV, (info.roughness)), 0).rg;
    uint specularTextureLevel = querySpecularTextureLevels(specularRad);
    float3 specularIBLTexture = shaderTextureCube[specularRad].SampleLevel(gsamAnisotoropicWrap, normalize(reflec), specularTextureLevel * (info.roughness)).rgb;
    float3 specularIrradiance = float3(0, 0, 0);

    specularIrradiance = lerp(rwTexture[ambient.specularOutput][uv].rgb, specularIBLTexture, pow(info.roughness.xxx, 2));
    //specularIrradiance = rwTexture[ambient.specularOutput][uv].rgb;
    
    //float3 specularIBL = EnvBRDFApprox(specularIrradiance, info.roughness, NDotV);
    specularIrradiance = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);
    //return float4(0, 0, 0, 1);
    //return float4(atten * (specularIBL), 1.0f);
    //float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
    //return float4(atten * (specularIBL), 1.0f);
    kD = float3(0, 0, 0);
    return float4((atten * kD * diffuseIrradiance + specularIrradiance), 1.0f);
}

#endif