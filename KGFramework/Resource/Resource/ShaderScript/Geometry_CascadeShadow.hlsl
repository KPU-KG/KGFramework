#include "Define_Global.hlsl"
#include "Define_GSCascadeCamera.hlsl"

#ifdef STATIC_MESH
    #include "Mesh_Static.hlsl"
#endif
#ifdef SKINNED_MESH
    #include "Mesh_Skinned.hlsl"
#endif

struct ShadowVSOutput
{
    float4 position : SV_Position;
};

struct ShadowGSOutput
{
    float4 position : SV_Position;
    uint renderIndex : SV_RenderTargetArrayIndex;
};


ShadowVSOutput VertexShaderFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    ShadowVSOutput result;
    float4x4 world = GetWorldMatrix(InstanceID, input);
    result.position = mul(float4(input.position, 1.0f), world);
    return result;
}

[maxvertexcount(18)]
void GeometryShaderFuction(triangle ShadowVSOutput inData[3], inout TriangleStream<ShadowGSOutput> outStream )
{
    for (int index = 0; index < 4; ++index)
    {
        ShadowGSOutput output;
        output.renderIndex = index;
        for (int v = 0; v < 3; v++)
        {
            output.position = mul(inData[v].position, mul(view[index], projection[index]));
            outStream.Append( output );
        }
        outStream.RestartStrip();
    }
}