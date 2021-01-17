#include "GlobalDefine.hlsl"
#include "DeferredDefine.hlsl"
#include "GBufferDefine.hlsl"


float4x4 inverse(float4x4 m)
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

struct MaterialData
{
    uint ColorTextureIndex;
    uint NormalTextureIndex;
    uint MetalicTextureIndex;
    uint RoughnessTextureIndex;
    float SpecularValue;
    float2 UVSize;
    float pad;
};

StructuredBuffer<MaterialData> materialData : register(t1);

struct VSOutput
{
    float4 position : SV_Position;
    float4 worldPosition : WPOSITION;
    float4 worldNormal : WNORMAL;
    float4 worldTangent : WTANGENT;
    float4 worldBiTangent : WBITANGENT;
    float2 uv : TEXCOORD;
    uint InstanceID : SV_InstanceID;
};

VSOutput DefaultVertexFuction(VertexData input, uint InstanceID : SV_InstanceID)
{
    VSOutput result;
    
    float4x4 animationMatrix = (float4x4) 0.0f;
    for (int i = 0; i < 1; i++)
    {
        uint animationIndex = (MAX_COUNT_BONE * InstanceID) + input.bone[i];
        //animationMatrix += input.weight[i] * mul(boneOffsetInfo[input.bone[i]], animationTransformInfo[animationIndex]);
        animationMatrix += mul((boneOffsetInfo[input.bone[i]]), animationTransformInfo[animationIndex]);
    }
    //float4x4 world = mul(animationMatrix, objectInfo[InstanceID].world);
    float4x4 world = animationMatrix;
    
    result.worldPosition = mul(float4(input.position, 1), world);
    result.position = mul(result.worldPosition, viewProjection);
    result.worldNormal = mul(float4(input.normal, 0), animationMatrix);
    result.worldTangent = mul(float4(input.tangent, 0), animationMatrix);
    result.worldBiTangent = mul(float4(input.bitangent, 0), animationMatrix);
    
    result.uv = input.uv;
    result.InstanceID = InstanceID;
    return result;
}

GBufferOut DefaultPixelFuction(VSOutput input)
{
    PixelResult result;
    MaterialData mat = materialData[objectInfo[input.InstanceID].materialIndex];
    
    float2x2 uvScale = float2x2(mat.UVSize.x, 0, 0, mat.UVSize.y);
    float2 uv = mul(input.uv, uvScale);
    
    
    result.albedo = shaderTexture[mat.ColorTextureIndex].Sample(gsamAnisotoropicWrap, uv).xyz;
    result.reflection = objectInfo[input.InstanceID].environmentMapIndex / 12000.0f;
    
    result.specular = 0.0f;
    result.metalic = shaderTexture[mat.MetalicTextureIndex].Sample(gsamAnisotoropicWrap, uv).xxx;
    result.roughness = shaderTexture[mat.RoughnessTextureIndex].Sample(gsamAnisotoropicWrap, uv).xxx;
    result.emssion = 0.0f;
    
    float3x3 TBN = float3x3(
        normalize(input.worldTangent.xyz),
        normalize(input.worldBiTangent.xyz),
        normalize(input.worldNormal.xyz)
    );
    //TBN = transpose(TBN);
    float3 normalMap = shaderTexture[mat.NormalTextureIndex].Sample(gsamAnisotoropicWrap, uv).xyz;
    normalMap = normalMap * 2 - 1.0f;
    result.wNormal = normalize(mul(normalMap, TBN));
    //result.wNormal = input.worldNormal;
    
    result.environmentMap = objectInfo[input.InstanceID].environmentMapIndex;
    result.reserve0 = 1.0f.xxx;
    
    GBufferOut bufferResult = PixelEncode(result);
    return bufferResult;
}

