#include "Define_Global.hlsl"
#include "Define_GBuffer.hlsl"

GBufferOut PixelShaderFunction(SurfaceInput input)
{
    Surface face;
    face.albedo = float3(0, 1, 0);
    face.reflection = 0.0f;
    
    face.specular = 0.0f;
    face.metalic = 0.0f;
    face.roughness = 0.0f;
    face.emssion = 0.0f;
    face.wNormal = float3(0, 1, 0);
    face.environmentMap = 0;
    face.reserve0 = 1.0f.xxx;
    
    GBufferOut bufferResult = PixelEncode(face);
    return bufferResult;
}