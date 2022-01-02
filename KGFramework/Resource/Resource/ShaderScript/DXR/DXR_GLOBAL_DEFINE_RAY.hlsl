#include "DXR_GLOBAL_DEFINE.hlsl"

#ifndef __DXR_HLSL_DEFINE_RAY__
#define __DXR_HLSL_DEFINE_RAY__

RaytracingAccelerationStructure scene : register(t2);

struct Built_in_attribute
{
    float BaryX;
    float BaryY;
};

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float3 HitAttribute(float3 vertexAttribute[3], Built_in_attribute attr)
{
    return vertexAttribute[0] +
        attr.BaryX * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.BaryY * (vertexAttribute[2] - vertexAttribute[0]);
}

float3 HitAttribute(float3 v0, float3 v1, float3 v2, Built_in_attribute attr)
{
    return v0 + attr.BaryX * (v1 - v0) + attr.BaryY * (v2 - v0);
}

float2 HitAttribute(float2 vertexAttribute[3], Built_in_attribute attr)
{
    return vertexAttribute[0] +
        attr.BaryX * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.BaryY * (vertexAttribute[2] - vertexAttribute[0]);
}

float2 HitAttribute(float2 v0, float2 v1, float2 v2, Built_in_attribute attr)
{
    return v0 + attr.BaryX * (v1 - v0) + attr.BaryY * (v2 - v0);
}

VertexData HitAttribute(VertexData vertex[3], Built_in_attribute attr)
{
    VertexData result;
    result.position = HitAttribute(vertex[0].position, vertex[1].position, vertex[2].position, attr);
    result.normal = HitAttribute(vertex[0].normal, vertex[1].normal, vertex[2].normal, attr);
    result.tangent = HitAttribute(vertex[0].tangent, vertex[1].tangent, vertex[2].tangent, attr);
    result.bitangent = HitAttribute(vertex[0].bitangent, vertex[1].bitangent, vertex[2].bitangent, attr);
    result.uv = HitAttribute(vertex[0].uv, vertex[1].uv, vertex[2].uv, attr);
    result.uv1 = HitAttribute(vertex[0].uv1, vertex[1].uv1, vertex[2].uv1, attr);
    
    float orientation = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE ? 1 : -1;
    result.normal *= orientation;
    result.tangent *= orientation;
    result.bitangent *= orientation;
    
    return result;
}

static const float FLT_MAX = asfloat(0x7F7FFFFF);

float4 TraceRadiance(float3 origin, float3 direction, uint recursionDepth, bool on = true)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0;
    ray.TMax = on ? FLT_MAX : 0;
    
    Payload payload;
    payload.color = float4(0,0,0,1);
    payload.recursionDepth = recursionDepth;
    TraceRay(
        scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        0xFF,
        0,
        2,
        0,
        ray,
        payload
    );
    return payload.color;
}

float4 TraceShadow(float3 origin, float3 direction, uint recursionDepth, bool on = true)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0;
    ray.TMax = on ? FLT_MAX : 0;
    
    Payload payload;
    payload.color = float4(0, 0, 0, 1);
    payload.recursionDepth = 30;
    TraceRay(
        scene,
        RAY_FLAG_SKIP_CLOSEST_HIT_SHADER | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        //RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
        0xFF,
        1,
        2,
        1,
        ray,
        payload
    );
    return payload.color;
}

float4 SampleLevelT(Texture2D tex, SamplerState sam, float2 uv)
{
    uint width, height, levels;
    tex.GetDimensions(0, width, height, levels);
    float currentLevel = levels * log10(RayTCurrent() / 250.0f);
    return tex.SampleLevel(sam, uv, currentLevel);
}

#endif