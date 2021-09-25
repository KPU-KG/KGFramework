#ifndef __DXR_HLSL_DEFINE__
#define __DXR_HLSL_DEFINE__

// Global Root Signature Part

struct _CameraData
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;

    float4x4 inverseView;
    float4x4 inverseProjection;
    float4x4 inverseViewProjection;

    float3 cameraWorldPosition;
    float1 pad0;
    float3 look;
    float1 pad1;
    double gameTime;
};

struct _AmbientData
{
    uint skyBoxId;
    uint iblLut;
    uint iblRad;
    uint iblIrrad;
};

StructuredBuffer<uint> instanceIndexBuffer : register(t0);
cbuffer CameraData : register(b0)
{
    _CameraData camera;
};

cbuffer AmbientData : register(b1)
{
    _AmbientData ambient;
};

RWTexture2D<float4> output : register(u0);
RaytracingAccelerationStructure scene : register(t2);
Texture2D<float4> shaderTexture[] : register(t0, space1);
Texture2DArray<float4> shaderTextureArray[] : register(t0, space2);
TextureCube<float4> shaderTextureCube[] : register(t0, space3);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotoropicWrap : register(s4);
SamplerState gsamAnisotoropicClamp : register(s5);
SamplerComparisonState gsamAnisotoropicCompClamp : register(s6);
SamplerComparisonState gsamLinerCompClamp : register(s7);

// Local Root Signature Part


struct VertexData
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    uint4 bone : BONE;
    float4 weight : WEIGHT;
};

struct Surface
{
    float3 albedo;
    float1 emssion;

    float1 specular;
    float1 metalic;
    float1 roughness;
    float ao;

    float3 wNormal;

    uint environmentMap;
    uint3 reserve0;
};

struct SurfaceInput
{
    float4 position;
    float4 worldPosition;
    float4 worldNormal;
    float4 worldTangent;
    float4 worldBiTangent;
    float2 uv;
    uint InstanceID;
};

Surface UserSurfaceFunction(SurfaceInput input);

StructuredBuffer<VertexData> vertexBuffer : register(t0, space4);
StructuredBuffer<uint> indexBuffer : register(t1, space4);

struct InstanceData
{
    float4x4 world;
    uint materialIndex;
    uint environmentMapIndex;
    uint2 padding;
};
StructuredBuffer<InstanceData> objectInfo : register(t2, space4);
//Vertex
//Index
//Object
//Material -> Surface

// Payload

struct Payload
{
    float4 color;
    uint recursionDepth;
};
static const uint maxRecursionDepth = 5;

struct Built_in_attribute
{
    float BaryX;
    float BaryY;
};

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float3 LinearToGamma(float3 rgb)
{
    return pow(rgb, 1 / 2.2);
}

float3 GammaToLinear(float3 rgb)
{
    return pow(rgb, 2.2);
}

float4 LinearToGamma(float4 rgb)
{
    return float4(pow(rgb.rgb, 1 / 2.2), rgb.a);
}

float4 GammaToLinear(float4 rgb)
{
    return float4(pow(rgb.rgb, 2.2), rgb.a);
}

float3 HitAttribute(float3 vertexAttribute[3], Built_in_attribute attr)
{
    return vertexAttribute[0] +
        attr.BaryX * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.BaryY * (vertexAttribute[2] - vertexAttribute[0]);
}
float3 HitAttribute(float3 v0, float3 v1 , float3 v2, Built_in_attribute attr)
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

float4 TraceRadiance(float3 origin, float3 direction, uint recursionDepth)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.0f;
    ray.TMax = 1000.0f;
    
    Payload payload;
    payload.color = 0;
    payload.recursionDepth = recursionDepth;
    TraceRay(
        scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        0xFF,
        0,
        1,
        0,
        ray,
        payload
    );
    return payload.color;
}

#endif