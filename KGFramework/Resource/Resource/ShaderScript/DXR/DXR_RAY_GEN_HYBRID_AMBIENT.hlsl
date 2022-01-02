#include "DXR_GLOBAL_DEFINE_RAY.hlsl"
#include "DXR_DEFINE_LIGHT.hlsl"
#include "DXR_UTILL_LIGHT_CUSTOM.hlsl"

float Pow2(float x)
{
    return x * x;
}

float2 Pow2(float2 x)
{
    return x * x;
}

float3 Pow2(float3 x)
{
    return x * x;
}

float4 Pow2(float4 x)
{
    return x * x;
}

float Pow3(float x)
{
    return x * x * x;
}

float2 Pow3(float2 x)
{
    return x * x * x;
}

float3 Pow3(float3 x)
{
    return x * x * x;
}

float4 Pow3(float4 x)
{
    return x * x * x;
}

float Pow4(float x)
{
    float xx = x * x;
    return xx * xx;
}

float2 Pow4(float2 x)
{
    float2 xx = x * x;
    return xx * xx;
}

float3 Pow4(float3 x)
{
    float3 xx = x * x;
    return xx * xx;
}

float4 Pow4(float4 x)
{
    float4 xx = x * x;
    return xx * xx;
}



// [ Duff et al. 2017, "Building an Orthonormal Basis, Revisited" ]
float3x3 GetTangentBasis(float3 TangentZ)
{
    const float Sign = TangentZ.z >= 0 ? 1 : -1;
    const float a = -rcp(Sign + TangentZ.z);
    const float b = TangentZ.x * TangentZ.y * a;
	
    float3 TangentX = { 1 + Sign * a * Pow2(TangentZ.x), Sign * b, -Sign * TangentZ.x };
    float3 TangentY = { b, Sign + a * Pow2(TangentZ.y), -TangentZ.y };

    return float3x3(TangentX, TangentY, TangentZ);
}

// [ Heitz 2018, "Sampling the GGX Distribution of Visible Normals" ]
// http://jcgt.org/published/0007/04/01/

float4 ImportanceSampleVisibleGGX(float2 E, float a2, float3 V)
{
	// TODO float2 alpha for anisotropic
    float a = sqrt(a2);

	// stretch
    float3 Vh = normalize(float3(a * V.xy, V.z));

	// Orthonormal basis
	// Tangent0 is orthogonal to N.
    float3 Tangent0 = (Vh.z < 0.9999) ? normalize(cross(float3(0, 0, 1), Vh)) : float3(1, 0, 0);
    float3 Tangent1 = cross(Vh, Tangent0);

    float Radius = sqrt(E.x);
    float Phi = 2 * PI * E.y;

    float2 p = Radius * float2(cos(Phi), sin(Phi));
    float s = 0.5 + 0.5 * Vh.z;
    p.y = (1 - s) * sqrt(1 - p.x * p.x) + s * p.y;

    float3 H;
    H = p.x * Tangent0;
    H += p.y * Tangent1;
    H += sqrt(saturate(1 - dot(p, p))) * Vh;

	// unstretch
    H = normalize(float3(a * H.xy, max(0.0, H.z)));

    float NoV = V.z;
    float NoH = H.z;
    float VoH = dot(V, H);

    float d = (NoH * a2 - NoH) * NoH + 1;
    float D = a2 / (PI * d * d);

    float G_SmithV = 2 * NoV / (NoV + sqrt(NoV * (NoV - NoV * a2) + a2));

    float PDF = G_SmithV * VoH * D / NoV;

    return float4(H, PDF);
}

float3 GenerateReflectedRayDirection(
	float3 IncidentDirection,
	float3 WorldNormal,
	float Roughness,
	float2 RandSample
)
{
    float3 RayDirection;
    if (Roughness < 0.001) //ReflectionSmoothClamp)
    {
        RayDirection = reflect(IncidentDirection, WorldNormal);
    }
    else
    {
        float3 N = WorldNormal;
        float3 V = -IncidentDirection;
        float2 E = RandSample;

        float3x3 TangentBasis = GetTangentBasis(N);
        float3 TangentV = mul(TangentBasis, V);

        float NoV = saturate(dot(V, WorldNormal));

        float4 Sample = ImportanceSampleVisibleGGX(E, Pow4(Roughness), TangentV);

        float3 H = mul(Sample.xyz, TangentBasis);
        float3 L = 2 * dot(V, H) * H - V;

        RayDirection = L;
    }

    return RayDirection;
}

#define SAMPLE_COUNT 1

[shader("raygeneration")]
void RayGeneration()
{
    int2 launchIndex = DispatchRaysIndex().xy;
    int2 dimensions = DispatchRaysDimensions().xy;
    float2 ndc = ((((float2) launchIndex + 0.5f) / (float2) dimensions) * 2.f - 1.f);
    ndc.y = -ndc.y;

    float4x4 inverseViewProj = mul(camera.inverseProjection, camera.inverseView);
    float4 unproj = mul(float4(ndc, 0, 1), inverseViewProj);
    float3 world = unproj.xyz / unproj.w;
    float depth = InputGBuffer4.Load(float3(launchIndex, 0)).x;
    float3 dir = normalize(world - camera.cameraWorldPosition);
    
    //output[launchIndex] = TraceRadiance(camera.cameraWorldPosition, normalize(world - camera.cameraWorldPosition), 0);
    
    float4 color = output[launchIndex];
    world = DepthToWorldPosition(depth, ndc, inverseViewProj);
    
    Surface surface = PixelDecode(
        InputGBuffer0.Load(float3(launchIndex, 0)),
        InputGBuffer1.Load(float3(launchIndex, 0)),
        InputGBuffer2.Load(float3(launchIndex, 0)),
        InputGBuffer3.Load(float3(launchIndex, 0))
    );
    
    float3 reflec = reflect(dir, surface.wNormal);

    float4 reflectColor = float4(0, 0, 0, 0);
    //float2 randSample = frac(float2(camera.gameTime, camera.gameTime));
    float2 randSample = launchIndex;
    uint SampleCount = 0;
    reflectColor = TraceRadiance(world, reflec, 1, depth != 1.0f);
    rwTexture[ambient.specularOutput][launchIndex] = reflectColor;
    //rwTexture[ambient.specularOutput][launchIndex] = TraceRadiance(world, normalize(reflec), 1, depth != 1.0f);
}