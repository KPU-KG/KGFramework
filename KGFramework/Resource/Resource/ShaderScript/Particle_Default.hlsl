#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"

struct ParticleData
{
    float3 position;
    float rotation;
    float3 speed;
    float rotationSpeed;
    float2 size;
    uint materialIndex;
    float pad1;
    float4 color;
    double startTime;
    float lifeTime;
    float pad0;
};

StructuredBuffer<ParticleData> particleInfo : register(t0);


struct ParticleVertexOutput
{
    float4 position : SV_Position;
    uint InstanceID : SV_InstanceID;
};

struct ParticleGSOutput
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

struct ParticlePixelInput
{
    float4 position : SV_Position;
    float4 projPosition : POSITION;
    uint InstanceID : SV_InstanceID;
};


//메테리얼 데이터 추가

ParticlePixelInput VertexShaderFunction(uint InstanceID : SV_InstanceID)
{
    ParticleVertexOutput result;
    result.position = float4(0, 0, 0, 1);
    result.InstanceID = InstanceID;
    return result;
}

static float2 uvs[4] = { float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0) };

[maxvertexcount(4)]
void GSBillBoard(point ParticleVertexOutput input[1], inout TriangleStream<ParticleGSOutput> outStream)
{
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = cross(up, look);
    
    ParticleData data = particleInfo[input[0].InstanceID];
    
    float halfW = data.size.x * 0.5f;
    float halfH = data.size.y * 0.5f;
    
    float3 animatedPosition = data.position;
    
    float4 pVertices[4];
    pVertices[0] = float4(animatedPosition + (halfW * right) - (halfH * up), 1.0f);
    pVertices[1] = float4(animatedPosition + (halfW * right) + (halfH * up), 1.0f);
    pVertices[2] = float4(animatedPosition - (halfW * right) - (halfH * up), 1.0f);
    pVertices[3] = float4(animatedPosition - (halfW * right) + (halfH * up), 1.0f);
    
    ParticleGSOutput output;
    for (int i = 0; i < 4; i++)
    {
        output.positionW = pVertices[i].xyz;
        output.position = mul(mul(pVertices[i], view), projection);
        output.uv = uvs[i];
        output.normal = normalize(-look);
        outStream.Append(output);
    }
    
}

float4 PixelShaderFunction(ParticleGSOutput input) : SV_Target0
{
    //픽셀 쉐이더 추가
    //리턴
}

