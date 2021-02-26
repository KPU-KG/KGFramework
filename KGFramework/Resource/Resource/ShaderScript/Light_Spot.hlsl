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
    float s = light.FalloffStart * 1.0f;
    //float sxy = (light.Phi * tan(light.Phi)) / 0.707106781f; // 0.707106781 == sqrt(0.5f)
    float sxy = s * sin(light.Phi/2.0f);
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
    //float4x4 rotation = float4x4
    //(
    //    right.x, up.x, dir.x, 0,
    //    right.y, up.y, dir.y, 0,
    //    right.z, up.z, dir.z, 0,
    //    0, 0, 0, 1
    //);
    
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

    float SinAngle = sin(lightInfo[quad[0].InstanceID].Phi);
    float CosAngle = cos(lightInfo[quad[0].InstanceID].Phi);
    
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
    float3 lightDirection = lightData.Direction;
    float3 vToLight = lightData.Position - calcWorldPosition;
    
    float distance = length(vToLight);
    
    float atten = CalcAttenuation(distance, lightData.FalloffStart, lightData.FalloffStart);
    float spotFactor = CalcSpotFactor(normalize(vToLight), lightData);
    float shadowFactor = 1.0f;
    // https://heinleinsgame.tistory.com/19
    //float shadowFactor = PointShadowPoissonPCF(lightDirection, lightData, shadowData, dot(normalize(lightDirection), normalize(pixelData.wNormal)));
    
    return CustomLightCalculator(lightData, pixelData, normalize(-vToLight), normalize(-cameraDirection), atten * spotFactor) * shadowFactor;
}

