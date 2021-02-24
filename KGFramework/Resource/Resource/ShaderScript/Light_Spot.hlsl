#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"
#include "Define_Light.hlsl"
#include "Utill_LightCustom.hlsl"


struct LightVertexOutput
{
    float4 position : SV_Position;
    uint InstanceID : SV_InstanceID;
};

struct LightHSConstantOutput
{
    float edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

static const float3 HemilDir[2] =
{
    float3(1.0f, 1.0f, 1.0f),
    float3(-1.0f, 1.0f, -1.0f)
};

struct LightHSOutput
{
    float3 HemilDir : POSITION;
    uint InstanceID : SV_InstanceID;
};

struct LightPixelInput
{
    float4 position : SV_Position;
    float4 projPosition : POSITION;
    uint InstanceID : SV_InstanceID;
};


float4x4 GetLightMatrix(LightData light)
{
    float s = light.FalloffEnd * 1.0f;
    float x = light.Position.x;
    float y = light.Position.y;
    float z = light.Position.z;
    return float4x4
    (
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0,
        x, y, z, 1
    );
}


LightVertexOutput VertexShaderFunction(uint InstanceID : SV_InstanceID)
{
    LightVertexOutput result;
    result.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    result.InstanceID = InstanceID;
    return result;
}

LightHSConstantOutput ConstantHS()
{
    LightHSConstantOutput output;
    float tessFactor = 18.0f;
    output.edges[0] = output.edges[1] = output.edges[2] = output.edges[3] = tessFactor;
    output.Inside[0] = output.Inside[1] = tessFactor;
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
LightHSOutput HullShaderFunction(InputPatch<LightVertexOutput, 1> input, uint PatchID : SV_PrimitiveID)
{
    LightHSOutput output;
    output.HemilDir = HemilDir[PatchID];
    output.InstanceID = input[0].InstanceID;
    return output;
}

[domain("quad")]
LightPixelInput DomainShaderFunction(LightHSConstantOutput constant, float2 uv : SV_DomainLocation, OutputPatch<LightHSOutput, 4> quad)
{
    float2 posClipSpace = uv.xy * 2.0f - 1.0f;
    
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);
    float3 normDir = normalize(float3(posClipSpace.xy, maxLen - 1.0f) * quad[0].HemilDir);
    
    float4 position = float4(normDir.xyz, 1.0f);
    LightPixelInput output;
    float4x4 LightProjection = GetLightMatrix(lightInfo[quad[0].InstanceID]);
    LightProjection = mul(LightProjection, view);
    LightProjection = mul(LightProjection, projection);
    
    output.position = mul(position, LightProjection);
    output.projPosition = output.position;
    output.InstanceID = quad[0].InstanceID;
    
    return output;
}

float4 PixelShaderFunction(LightPixelInput input) : SV_Target0
{
    input.projPosition.xy /= input.projPosition.w;
    float2 uv = ProjPositionToUV(input.projPosition.xy);
    Surface pixelData = PixelDecode(
    InputGBuffer0.Sample(gsamPointWrap, uv),
    InputGBuffer1.Sample(gsamPointWrap, uv),
    InputGBuffer2.Sample(gsamPointWrap, uv),
    InputGBuffer3.Sample(gsamPointWrap, uv)
    );
    float depth = InputGBuffer4.Sample(gsamPointWrap, uv).x;
    
    LightData lightData = lightInfo[input.InstanceID];
    ShadowData shadowData = shadowInfo[input.InstanceID];
    float3 calcWorldPosition = DepthToWorldPosition(depth, input.projPosition.xy, mul(inverseProjection, inverseView));
    
    //float3 cameraDirection = look;
    float3 cameraDirection = calcWorldPosition - cameraWorldPosition;
    float3 lightDirection = calcWorldPosition - lightData.Position;
    
    float distance = length(lightDirection);
    
    float atten = CalcAttenuation(distance, lightData.FalloffStart, lightData.FalloffEnd);

    float shadowFactor = PointShadowPoissonPCF(lightDirection, lightData, shadowData, dot(normalize(lightDirection), normalize(pixelData.wNormal)));
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightDirection), normalize(-cameraDirection), atten) * shadowFactor;
}

