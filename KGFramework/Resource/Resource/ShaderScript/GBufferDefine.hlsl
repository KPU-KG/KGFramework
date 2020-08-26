struct PixelResult
{
    float3 albedo;
    float1 reserve0;
    
    float1 specular;
    float1 metailic;
    float1 roughness;
    float1 emssion;
    
    float3 wNormal;
    float1 reflection;
    
    float3 light;
    float1 reserve1;
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



GBufferOut PixelEncode(PixelResult pix)
{
    GBufferOut result;
    result.gbuffer0.xyz = pix.albedo;
    result.gbuffer0.w = pix.reserve0;
    
    result.gbuffer1.x = pix.specular;
    result.gbuffer1.y = pix.metailic;
    result.gbuffer1.z = pix.roughness;
    result.gbuffer1.w = pix.emssion;
    
    result.gbuffer2.xyz = SNORMTOUNORM(pix.wNormal);
    result.gbuffer2.w = pix.reflection;
    
    result.gbuffer3.xyz = pix.light;
    result.gbuffer3.w = pix.reserve1;
    
    return result;
}

PixelResult PixelDecode(float4 gbuffer0, float4 gbuffer1, float4 gbuffer2, float4 gbuffer3)
{
    PixelResult result;
    result.albedo = gbuffer0.xyz;
    result.reserve0 = gbuffer0.w;
    
    result.specular =gbuffer1.x;
    result.metailic = gbuffer1.y;
    result.roughness = gbuffer1.z;
    result.emssion = gbuffer1.w;
    
    
    result.wNormal = UNORMTOSNORM(gbuffer2.xyz);
    result.reflection = gbuffer2.w;
    
    result.light = gbuffer3.xyz;
    result.reserve1 = gbuffer3.w;
    
    return result;
}