#ifndef __GBUFFER_DEFINE__
#define __GBUFFER_DEFINE__
#include "Define_Global.hlsl"
struct GBufferOut
{
    float4 gbuffer0 : SV_Target0;
    float4 gbuffer1 : SV_Target1;
    float4 gbuffer2 : SV_Target2;
    uint4 gbuffer3 : SV_Target3;
};

float3 SNORMTOUNORM(float3 normal)
{
    normal *= 0.5f;
    normal += 0.5f;
    return normal;
}

float3 UNORMTOSNORM(float3 normal)
{
    normal -= 0.5f;
    normal *= 2.0f;
    return normal;
}

// http://c0de517e.blogspot.com/2015/01/notes-on-g-buffer-normal-encodings.html
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
float2 OctWrap(float2 v)
{
    return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}
 
float2 EncodeNormal(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}
 
float3 DecodeNormal(float2 f)
{
    f = f * 2.0 - 1.0;
 
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0 ? -t : t;
    return normalize(n);
}


GBufferOut PixelEncode(Surface surface)
{
    GBufferOut result;
    result.gbuffer0.xyz = surface.albedo;
    //result.gbuffer0.w = surface.reflection;
    result.gbuffer0.w = 1;
    
    result.gbuffer1.x = surface.specular;
    result.gbuffer1.y = surface.metalic;
    result.gbuffer1.z = surface.roughness;
    result.gbuffer1.w = 1;
    //result.gbuffer1.w = surface.emssion;
    
    result.gbuffer2.xy = EncodeNormal(surface.wNormal);
    
    result.gbuffer3.x = surface.environmentMap;
    //result.gbuffer3.yzw = surface.reserve0;
    result.gbuffer3.yzw = float3(0, 0, 1);
    
    return result;
}

Surface PixelDecode(float4 gbuffer0, float4 gbuffer1, float4 gbuffer2, uint4 gbuffer3)
{
    Surface result;
    result.albedo = gbuffer0.xyz;
    result.emssion = gbuffer0.w;
    
    result.specular =gbuffer1.x;
    result.metalic = gbuffer1.y;
    result.roughness = gbuffer1.z;
    result.ao = gbuffer1.w;
    
    
    result.wNormal = normalize(DecodeNormal(gbuffer2.xy));
    
    result.environmentMap = gbuffer3.x;
    result.reserve0 = gbuffer3.yzw;
    
    return result;
}
#endif // __DEPENDENCY_HLSL__
