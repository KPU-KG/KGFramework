#include "Define_Global.hlsl"

struct ShadowPSInput
{
    float4 position : SV_Position;
};

float4 PixelShaderFunction( ShadowPSInput outStream ) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}