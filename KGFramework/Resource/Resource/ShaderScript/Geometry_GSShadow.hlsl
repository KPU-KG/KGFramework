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


ShadowVSOutput VertexShaderFunction(VertexData input, uint InstanceID : SV_InstanceID)
{
    ShadowVSOutput result;
    float4x4 world = GetWorldMatrix(InstanceID, input);
    result.position = mul(float4(input.position, 1.0f), world);
    return result;
}

[maxvertexcount(18)]
void GeometryShaderFunction(triangle ShadowVSOutput inData[3], inout TriangleStream<ShadowGSOutput> outStream )
{
    for (int index = 0; index < 6; ++index)
    {
        ShadowGSOutput output;
        output.renderIndex = index;
        for (int v = 0; v < 3; v++)
        {
            output.position = mul(inData[v].position, mul(view[index], projection));
            outStream.Append( output );
        }
        outStream.RestartStrip();
    }
}