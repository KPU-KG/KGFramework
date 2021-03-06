#ifndef __GLOBAL_DEFINE__
#define __GLOBAL_DEFINE__

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
    float4 position : SV_Position;
    float4 worldPosition : WPOSITION;
    float4 worldNormal : WNORMAL;
    float4 worldTangent : WTANGENT;
    float4 worldBiTangent : WBITANGENT;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

Surface UserSurfaceFunction(SurfaceInput input);
float4x4 GetWorldMatrix(uint instanceID, VertexData vertex);

struct InstanceData
{
    float4x4 world;
    uint materialIndex;
    uint environmentMapIndex;
    uint2 padding;
};

StructuredBuffer<InstanceData> objectInfo : register(t0);

Texture2D<float4> shaderTexture[] : register(t0, space1);

Texture2DArray<float4> shaderTextureArray[] : register(t0, space5);

TextureCube<float4> shaderTextureCube[] : register(t0, space2);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotoropicWrap : register(s4);
SamplerState gsamAnisotoropicClamp : register(s5);
SamplerComparisonState gsamAnisotoropicCompClamp : register(s6);
SamplerComparisonState gsamLinerCompClamp : register(s7);

float3 LinearToGamma(float3 rgb)
{
    return pow(rgb, 1 / 2.2);
}

float3 GammaToLinear(float3 rgb)
{
    return pow(rgb, 2.2);
}

#endif
