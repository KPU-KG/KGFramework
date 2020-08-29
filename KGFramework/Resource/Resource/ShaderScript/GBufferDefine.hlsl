#ifndef __GBUFFER_DEFINE__
#define __GBUFFER_DEFINE__
struct PixelResult
{
    float3 albedo;
    float1 reflection;
    
    float1 specular;
    float1 metailic;
    float1 roughness;
    float1 emssion;
    
    float3 wNormal;
    
    float3 light;
    float1 reserve0;
};

struct GBufferOut
{
    float4 gbuffer0 : SV_Target0;
    float4 gbuffer1 : SV_Target1;
    float4 gbuffer2 : SV_Target2;
    float4 gbuffer3 : SV_Target3;
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


GBufferOut PixelEncode(PixelResult pix)
{
    GBufferOut result;
    result.gbuffer0.xyz = pix.albedo;
    result.gbuffer0.w = pix.reflection;
    
    result.gbuffer1.x = pix.specular;
    result.gbuffer1.y = pix.metailic;
    result.gbuffer1.z = pix.roughness;
    result.gbuffer1.w = pix.emssion;
    
    result.gbuffer2.xy = EncodeNormal(pix.wNormal);
    
    result.gbuffer3.xyz = pix.light;
    result.gbuffer3.w = pix.reserve0;
    
    return result;
}

PixelResult PixelDecode(float4 gbuffer0, float4 gbuffer1, float4 gbuffer2, float4 gbuffer3)
{
    PixelResult result;
    result.albedo = gbuffer0.xyz;
    result.reflection = gbuffer0.w;
    
    result.specular =gbuffer1.x;
    result.metailic = gbuffer1.y;
    result.roughness = gbuffer1.z;
    result.emssion = gbuffer1.w;
    
    
    result.wNormal = normalize(DecodeNormal(gbuffer2.xy));
    
    result.light = gbuffer3.xyz;
    result.reserve0 = gbuffer3.w;
    
    return result;
}
#endif // __DEPENDENCY_HLSL__
