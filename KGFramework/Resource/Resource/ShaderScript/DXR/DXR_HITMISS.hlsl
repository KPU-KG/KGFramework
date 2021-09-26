#include "DXR_GLOBAL_DEFINE.hlsl"
#include "DXR_DEFINE_LIGHT.hlsl"
#include "DXR_UTILL_LIGHT_CUSTOM.hlsl"

#ifndef __DXR_HLSL_HIT_MISS__
#define __DXR_HLSL_HIT_MISS__

//uint InstanceID()
//{
//    return 0;
//}

[shader("closesthit")]
void Hit(inout Payload payload : SV_Payload, Built_in_attribute attr)
{
    uint startIndex = 0;
    startIndex = PrimitiveIndex() * 3;
    const uint3 indices = { indexBuffer[startIndex], indexBuffer[startIndex + 1], indexBuffer[startIndex + 2] };
    VertexData vertices[3] = {
        vertexBuffer[indices[0]],
        vertexBuffer[indices[1]],
        vertexBuffer[indices[2]]
    };
    VertexData v = HitAttribute(vertices, attr);

    SurfaceInput input;
    input.position = float4(v.position, 1);
    input.worldPosition = float4(HitWorldPosition(), 1);
    input.worldNormal = mul(float4(v.normal, 0), objectInfo[instanceIndexBuffer[InstanceID()]].world);
    input.worldTangent = mul(float4(v.tangent, 0), objectInfo[instanceIndexBuffer[InstanceID()]].world);
    input.worldBiTangent = mul(float4(v.bitangent, 0), objectInfo[instanceIndexBuffer[InstanceID()]].world);
    input.uv = v.uv;
    input.InstanceID = instanceIndexBuffer[InstanceID()];
    Surface surface = UserSurfaceFunction(input);
    
    for (int i = 0; i < 1; ++i)
    {
        if(lightInfo[i].lightType.x == 0)
        {
            float4 shadow = float4(1,1,1, 1);
            shadow = TraceShadow(HitWorldPosition(), normalize(-lightInfo[i].Direction), 0);
            shadow = clamp(shadow, float4(0.3, 0.3, 0.3, 1.0f), float4(1, 1, 1, 1));
            payload.color += CustomLightCalculator(lightInfo[i], surface, normalize(lightInfo[i].Direction), normalize(-WorldRayDirection()), 1.0) * shadow;
        }
    }
    //payload.color += CustomAmbientLightCalculator(lightInfo[i], surface, normalize(lightInfo[i].Direction), normalize(WorldRayDirection()), 1.0, ambient.iblLut, ambient.iblIrrad, ambient.iblRad);
    payload.color += CustomAmbientLightCalculator(lightInfo[0], surface, float3(0, 1, 0), WorldRayDirection(), 1.0f, ambient.iblLut, ambient.iblIrrad, ambient.iblRad, payload.recursionDepth);
    //ambient.iblLut, ambient.iblRad, ambient.iblIrrad);

}

[shader("miss")]
void Miss(inout Payload payload : SV_Payload)
{
    payload.color = GammaToLinear(shaderTextureCube[ambient.skyBoxId].SampleLevel(gsamAnisotoropicWrap, WorldRayDirection(), 0));
}

#endif 

