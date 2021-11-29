
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



RWStructuredBuffer<VertexData> resultVertex : register(u0);
StructuredBuffer<VertexData> sourceVertex : register(t0);
StructuredBuffer<float4x4> boneOffsetInfo : register(t1);
StructuredBuffer<float4x4> animationTransformInfo : register(t2);
cbuffer inst : register(b0)
{
    int instanceID;
};
cbuffer inst : register(b1)
{
    float4x4 instanceWorld;
};


#define MAX_COUNT_BONE 64

float4x4 GetWorldMatrix(uint instanceID, VertexData vertex)
{
    float4x4 animationMatrix = (float4x4) 0.0f;
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        int animationIndex = (MAX_COUNT_BONE * instanceID) + vertex.bone[i];
        animationMatrix += vertex.weight[i] * mul(boneOffsetInfo[vertex.bone[i]], animationTransformInfo[animationIndex]);
    }
    return animationMatrix;
}

[numthreads(32, 1, 1)]
void ComputeShaderFunction(uint3 groupId : SV_GroupID, int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x; 
    VertexData newVert = sourceVertex[index];
    float4x4 world = mul(GetWorldMatrix(instanceID, sourceVertex[index]), instanceWorld);
    newVert.position = mul(float4(sourceVertex[index].position, 1), world).xyz;
    newVert.normal = mul(float4(sourceVertex[index].normal, 1), world).xyz;
    newVert.tangent = mul(float4(sourceVertex[index].tangent, 1), world).xyz;
    newVert.bitangent = mul(float4(sourceVertex[index].bitangent, 1), world).xyz;
    resultVertex[index] = newVert;
}