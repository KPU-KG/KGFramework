#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"

struct UIData
{
    float2 position;
    float2 size;
    float2 leftTopUV;
    float2 rightBottomUV;
    float4 color;
    float rotationAngle;
    float progressValue;
    float depth;
    float pad0;
    uint parentPivot;
    uint localPivot;
    uint progressShape;
    uint materialIndex;
};

StructuredBuffer<UIData> uiInfo : register(t0);


// Write User Material Data Foramt
struct MaterialData
{
    uint ColorTextureIndex;
};

// Don't Touch This Line
StructuredBuffer<MaterialData> materialData : register(t1);

struct ParticleVertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct ParticleVertexOutput
{
    float4 position : SV_Position;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

struct ParticleGSOutput
{
    float4 position : SV_Position;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

//메테리얼 데이터 추가

ParticleVertexOutput VertexShaderFunction(ParticleVertexInput input, uint InstanceID : SV_InstanceID)
{
    ParticleVertexOutput result;
    result.position = float4(0, 0, 0, 1);
    result.positionW = float3(0, 0, 0);
    result.normal = float3(0, 0, 0);
    result.uv = float2(0, 0);
    result.InstanceID = InstanceID;
    return result;
}

static float2 uvs[4] = { float2(1, 1), float2(1, 0), float2(0, 1), float2(0, 0) };
static float2 parentPivotUv[9] ={
    float2(+0, +1),
    float2(+0, +0),
    float2(+0, -1),
    float2(-1, +1),
    float2(-1, +0),
    float2(-1, -1),
    float2(+1, +1),
    float2(+1, +0),
    float2(+1, -1),
};

[maxvertexcount(4)]
void GeometryShaderFunction(point ParticleVertexOutput input[1], inout TriangleStream<ParticleGSOutput> outStream)
{
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = float3(1,0,0);
    
    UIData data = uiInfo[input[0].InstanceID];
    MaterialData mat = materialData[data.materialIndex];
    
    float halfW = data.size.x * 0.5f;
    float halfH = data.size.y * 0.5f;
    
    float3 animatedPosition = float3(parentPivotUv[data.parentPivot] - (parentPivotUv[data.localPivot] * (data.size * 0.5f)) + data.position.xy, 0.0f);
    
    float4 pVertices[4];
    pVertices[0] = float4(animatedPosition +  (halfW * right) - (halfH * up), 1.0f);
    pVertices[1] = float4(animatedPosition + (halfW * right) + (halfH * up), 1.0f);
    pVertices[2] = float4(animatedPosition - (halfW * right) - (halfH * up), 1.0f);
    pVertices[3] = float4(animatedPosition - (halfW * right) + (halfH * up), 1.0f);
    
    ParticleGSOutput output;
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pVertices[i].xyz;
        output.position = float4(pVertices[i].xyz, 1);
        //output.position = mul(pVertices[i], viewProjection);
        output.uv = uvs[i];
        output.normal = normalize(-look);
        output.InstanceID = input[0].InstanceID;
        outStream.Append(output);
    }
}

float4 PixelShaderFunction(ParticleGSOutput input) : SV_Target0
{
    UIData data = uiInfo[input.InstanceID];
    MaterialData mat = materialData[data.materialIndex];
    clip(1 - input.uv.r - (1 - data.progressValue));
    return shaderTexture[mat.ColorTextureIndex].Sample(gsamAnisotoropicClamp, input.uv) * data.color;
}

