#include "Define_Global.hlsl"
#include "Define_GSCamera.hlsl"

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
    result.position = mul(float4(input.position, 1.0f), objectInfo[InstanceID].world);
    return result;
}

[maxvertexcount(18)]
void GeometryShaderFuction(triangle ShadowVSOutput inData[3], inout TriangleStream<ShadowGSOutput> outStream )
{
    for (int index = 0; index < 6; ++index)
    {
        ShadowGSOutput output;
        output.renderIndex = inData;
        for (int v = 0; v < 3; v++)
        {
            output.position = mul(inData[v].position, viewProjection[index]);
            outStream.Append( output );
        }
        outStream.RestartStrip();
    }
}