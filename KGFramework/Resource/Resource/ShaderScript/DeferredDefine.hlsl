
struct PixelResult
{
    float3 diffuse;
    float1 reserve0;
    
    float3 specular;
    float1 roughness;
    
    float3 wNormal;
    float1 reflection;
    
    float3 light;
    float1 emission;
};

struct GBufferOut
{
    float4 gbuffer0 : SV_Target0;
    float4 gbuffer1 : SV_Target1;
    float4 gbuffer2 : SV_Target2;
    float4 gbuffer3 : SV_Target3;
};

GBufferOut PixelEncode(PixelResult pix)
{
    GBufferOut result;
    result.gbuffer0.xyz = pix.diffuse;
    result.gbuffer0.w = pix.reserve0;
    
    result.gbuffer1.xyz = pix.specular;
    result.gbuffer1.w = pix.roughness;
    
    result.gbuffer2.xyz = pix.wNormal;
    result.gbuffer2.w = pix.reflection;
    
    result.gbuffer3.x = pix.light;
    result.gbuffer3.w = pix.emission;
    
    return result;
}

PixelResult PixelDecode(GBufferOut pix)
{
    PixelResult result;
    
    result.diffuse = pix.gbuffer0.xyz;
    result.reserve0 = pix.gbuffer0.w;
    result.specular = pix.gbuffer1.xyz;
    result.roughness = pix.gbuffer1.w;
    result.wNormal = pix.gbuffer2.xyz;
    result.reflection = pix.gbuffer2.w;
    result.light = pix.gbuffer3.xyz;
    result.emission = pix.gbuffer3.w;
    
    return result;
}