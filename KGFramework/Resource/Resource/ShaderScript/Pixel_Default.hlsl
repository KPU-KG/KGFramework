#include "Define_Global.hlsl"
#include "Define_GBuffer.hlsl"

GBufferOut PixelShaderFunction(SurfaceInput input)
{
    Surface surface = UserSurfaceFunction(input);
    GBufferOut bufferResult = PixelEncode(surface);
    return bufferResult;
}