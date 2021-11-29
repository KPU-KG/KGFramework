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
    uint specularOutput;
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
RWTexture2D<float4> rwTexture[] : register(u1);
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

Texture2D InputGBuffer0 : register(t3, space0);
Texture2D InputGBuffer1 : register(t4, space0);
Texture2D InputGBuffer2 : register(t5, space0);
Texture2D<uint4> InputGBuffer3 : register(t6, space0);
Texture2D InputGBuffer4 : register(t7, space0);
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

static const uint maxRecursionDepth = 1;


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

Surface PixelDecode(float4 gbuffer0, float4 gbuffer1, float4 gbuffer2, uint4 gbuffer3)
{
    Surface result;
    result.albedo = gbuffer0.xyz;
    result.emssion = gbuffer0.w;
    
    result.specular = gbuffer1.x;
    result.metalic = gbuffer1.y;
    result.roughness = gbuffer1.z;
    result.ao = gbuffer1.w;
    
    
    result.wNormal = normalize(DecodeNormal(gbuffer2.xy));
    
    result.environmentMap = gbuffer3.x;
    result.reserve0 = gbuffer3.yzw;
    
    return result;
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

#endif