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


// Write User Material Data Foramt
struct MaterialData
{
    uint ColorTextureIndex;
    float2 animatedCounts;
    float framePerSecond;
    float3 globalAcc;
    float isRepeat;
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
    result.positionW = float3(0,0,0);
    result.normal = float3(0, 0, 0);
    result.uv = float2(0, 0);
    result.InstanceID = InstanceID;
    return result;
}

static float2 uvs[4] = { float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0) };

[maxvertexcount(4)]
void GeometryShaderFunction(point ParticleVertexOutput input[1], inout TriangleStream<ParticleGSOutput> outStream)
{
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = normalize(cross(up, look));
    up = normalize(cross(look, right));
    
    ParticleData data = particleInfo[input[0].InstanceID];
    MaterialData mat = materialData[data.materialIndex];
    
    float halfW = data.size.x * 0.5f;
    float halfH = data.size.y * 0.5f;
    
    double deltaTime = gameTime - data.startTime;
    
    float3 animatedPosition = data.position + data.speed * deltaTime + 0.5 * mat.globalAcc * pow(deltaTime, 2);
    
    float4 pVertices[4];
    pVertices[0] = float4(animatedPosition + (halfW * right) - (halfH * up), 1.0f);
    pVertices[1] = float4(animatedPosition + (halfW * right) + (halfH * up), 1.0f);
    pVertices[2] = float4(animatedPosition - (halfW * right) - (halfH * up), 1.0f);
    pVertices[3] = float4(animatedPosition - (halfW * right) + (halfH * up), 1.0f);
    
    ParticleGSOutput output;
    if (data.startTime + data.lifeTime > gameTime)
    {
        for (int i = 0; i < 4; i++)
        {
            output.positionW = pVertices[i].xyz;
            output.position = mul(pVertices[i], viewProjection);
            output.uv = uvs[i];
            output.normal = normalize(-look);
            output.InstanceID = input[0].InstanceID;
            outStream.Append(output);
        }
    }
    
}

float4 PixelShaderFunction(ParticleGSOutput input) : SV_Target0
{
    ParticleData data = particleInfo[input.InstanceID];
    MaterialData mat = materialData[data.materialIndex];
    return shaderTexture[mat.ColorTextureIndex].Sample(gsamAnisotoropicClamp, input.uv) * data.color;
}

