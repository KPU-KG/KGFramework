#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"

#define STATIC_MESH

#ifdef STATIC_MESH
    #include "Mesh_Static.hlsl"
#endif
#ifdef SKINNED_MESH
    #include "Mesh_Skinned.hlsl"
#endif

SurfaceInput VertexShaderFunction(VertexData input, uint InstanceID : SV_InstanceID)
{
    SurfaceInput result;
    float4x4 worldMatrix = GetWorldMatrix(InstanceID, input);
    result.worldPosition = mul(float4(input.position, 1), worldMatrix);
    result.position = mul(result.worldPosition, viewProjection);
    result.worldNormal = mul(float4(input.normal, 0), worldMatrix);
    result.worldTangent = mul(float4(input.tangent, 0), worldMatrix);
    result.worldBiTangent = mul(float4(input.bitangent, 0), worldMatrix);
    result.uv = input.uv;
    result.InstanceID = InstanceID;
    
    return result;
}