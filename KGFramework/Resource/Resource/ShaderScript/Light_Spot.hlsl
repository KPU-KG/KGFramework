#include "Define_Global.hlsl"
#include "Define_NormalCamera.hlsl"
#include "Define_Light.hlsl"
#include "Utill_LightCustom.hlsl"


Texture2D<float> shadowArray[] : register(t0, space1);

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
    float s = light.FalloffStart * 1.1f;
    //float sxy = (light.Phi * tan(light.Phi)) / 0.707106781f; // 0.707106781 == sqrt(0.5f)
    float sxy = s * sin(light.Phi / 2.0f);
    float x = light.Position.x;
    float y = light.Position.y;
    float z = light.Position.z;
    float3 up = normalize(light.Up);
    //float3 up = float3(0, 1, 0);
    float3 dir = normalize(light.Direction);
    float3 right = cross(up, dir);
    //up = cross(dir, right);
    float4x4 scale = float4x4
    (
        sxy, 0, 0, 0,
        0, sxy, 0, 0,
        0, 0, s, 0,
        0, 0, 0, 1
    );
    
    float4x4 rotation = float4x4
    (
        right.x, right.y, right.z, 0,
        up.x, up.y, up.z, 0,
        dir.x, dir.y, dir.z, 0,
        0, 0, 0, 1
    );
    float4x4 translation = float4x4
    (
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    );
    return mul(mul(scale, rotation), translation);
}

float SpotLightShadowPoissonPCF(float3 worldPosition, LightData lightData, ShadowData shadowData, float cosTheta)
{

    float2 uv = float2(1.0f, 1.0f);
    float depth = 1.0f;
    float4 projPos = mul(float4(worldPosition, 1.0f), shadowData.shadowMatrix[0]);
    float3 projPos3 = projPos.xyz / projPos.w;
    uv = ProjPositionToUV(projPos3.xy);
    depth = projPos3.z;

    float bias = 0.0005f * tan(acos(cosTheta));
    bias = clamp(bias, 0.00001f, 0.01f);

    
    static float2 poissonDisk[16] =
    {
        float2(-0.94201624, -0.39906216),
        float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870),
        float2(0.34495938, 0.29387760),
        
        float2(-0.91588581, 0.45771432),
        float2(-0.81544232, -0.87912464),
        float2(-0.38277543, 0.27676845),
        float2(0.97484398, 0.75648379),
        
        float2(0.44323325, -0.97511554),
        float2(0.53742981, -0.47373420),
        float2(-0.26496911, -0.41893023),
        float2(0.79197514, 0.19090188),
        
        float2(-0.24188840, 0.99706507),
        float2(-0.81409955, 0.91437590),
        float2(0.19984126, 0.78641367),
        float2(0.14383161, -0.14100790)
    };
    float result = 0.0f;
    for (uint n = 0; n < 16; n++)
    {
        result += shadowArray[shadowData.shadowMapIndex[0]].SampleCmpLevelZero(gsamAnisotoropicCompClamp, float2(uv + (poissonDisk[n] / 1200.0f)), depth - bias);
    }
    return result;
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
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
LightHSOutput HullShaderFunction(InputPatch<LightVertexOutput, 1> input, uint PatchID : SV_PrimitiveID)
{
    LightHSOutput output;
    output.InstanceID = input[0].InstanceID;
    return output;
}

[domain("quad")]
LightPixelInput DomainShaderFunction(LightHSConstantOutput constant, float2 uv : SV_DomainLocation, OutputPatch<LightHSOutput, 4> quad)
{
    static float CylinderPortion = 0.2f;
    static float ExpendAmount = (1.0f + CylinderPortion);

    float SinAngle = sin(lightInfo[quad[0].InstanceID].Phi / 2.0f);
    float CosAngle = cos(lightInfo[quad[0].InstanceID].Phi / 2.0f);
    
	// Transform the UV's into clip-space
    float2 posClipSpace = uv.xy * 2.0f + -1.0f;
    //float2 posClipSpace = uv.xy * float2(2.0, -2.0) + float2(-1.0, 1.0);

	// Find the vertex offsets based on the UV
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Force the cone vertices to the mesh edge
    float2 posClipSpaceNoCylAbs = saturate(posClipSpaceAbs * ExpendAmount);
    float maxLenNoCapsule = max(posClipSpaceNoCylAbs.x, posClipSpaceNoCylAbs.y);
    float2 posClipSpaceNoCyl = sign(posClipSpace.xy) * posClipSpaceNoCylAbs;

	// Convert the positions to half sphere with the cone vertices on the edge
    float3 halfSpherePos = normalize(float3(posClipSpaceNoCyl.xy, 1.0 - maxLenNoCapsule));

	// Scale the sphere to the size of the cones rounded base
    halfSpherePos = normalize(float3(halfSpherePos.xy * SinAngle, CosAngle));

	// Find the offsets for the cone vertices (0 for cone base)
    float cylinderOffsetZ = saturate((maxLen * ExpendAmount - 1.0) / CylinderPortion);

	// Offset the cone vertices to thier final position
    float4 posLS = float4(halfSpherePos.xy * (1.0 - cylinderOffsetZ), halfSpherePos.z - cylinderOffsetZ * CosAngle, 1.0);
    LightPixelInput output;
    
    float4x4 LightProjection = GetLightMatrix(lightInfo[quad[0].InstanceID]);
    LightProjection = mul(LightProjection, mul(view, projection));
    output.position = mul(posLS, LightProjection);
    output.projPosition = output.position;
    output.InstanceID = quad[0].InstanceID;
    
    return output;
}

float4 PixelShaderFunction(LightPixelInput input) : SV_Target0
{
    //return float4(0, 1, 0, 1);
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
    
    float atten = CalcAttenuation(distance, lightData.FalloffStart, lightData.FalloffStart);
    float spotFactor = CalcSpotFactor(normalize(-lightDirection), lightData);
    float shadowFactor = SpotLightShadowPoissonPCF(calcWorldPosition, lightData, shadowData, dot(normalize(lightDirection), normalize(pixelData.wNormal)));
    
    return CustomLightCalculator(lightData, pixelData, normalize(lightDirection), normalize(-cameraDirection), atten * spotFactor) * shadowFactor;
}

