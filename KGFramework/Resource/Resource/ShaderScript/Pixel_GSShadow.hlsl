#include "Define_Global.hlsl"
#include "Geometry_GSShadow.hlsl"

float4 PixelShaderFuction( ShadowGSOutput outStream ) : SV_TARGET
{
    return float4(0, 0, 0, 1);
}