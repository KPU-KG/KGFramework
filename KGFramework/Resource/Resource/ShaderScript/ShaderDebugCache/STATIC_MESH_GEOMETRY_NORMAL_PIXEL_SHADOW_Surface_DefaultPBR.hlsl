






struct VertexData 
{ 
    float3 position : POSITION ; 
    float3 normal : NORMAL ; 
    float3 tangent : TANGENT ; 
    float3 bitangent : BITANGENT ; 
    float2 uv : TEXCOORD0 ; 
    float2 uv1 : TEXCOORD1 ; 
    uint4 bone : BONE ; 
    float4 weight : WEIGHT ; 
} ; 

struct Surface 
{ 
    float3 albedo ; 
    float reflection ; 
    
    float1 specular ; 
    float1 metalic ; 
    float1 roughness ; 
    float1 emssion ; 
    
    float3 wNormal ; 
    
    uint environmentMap ; 
    uint3 reserve0 ; 
} ; 

struct SurfaceInput 
{ 
    float4 position : SV_Position ; 
    float4 worldPosition : WPOSITION ; 
    float4 worldNormal : WNORMAL ; 
    float4 worldTangent : WTANGENT ; 
    float4 worldBiTangent : WBITANGENT ; 
    float2 uv : TEXCOORD ; 
    uint InstanceID : SV_InstanceID ; 
} ; 

Surface UserSurfaceFunction ( SurfaceInput input ) ; 
float4x4 GetWorldMatrix ( uint instanceID , VertexData vertex ) ; 

struct InstanceData 
{ 
    float4x4 world ; 
    uint materialIndex ; 
    uint environmentMapIndex ; 
    uint2 padding ; 
} ; 

StructuredBuffer < InstanceData > objectInfo : register ( t0 ) ; 

Texture2D < float4 > shaderTexture [ ] : register ( t0 , space1 ) ; 

Texture2DArray < float4 > shaderTextureArray [ ] : register ( t0 , space1 ) ; 

TextureCube < float4 > shaderTextureCube [ ] : register ( t0 , space1 ) ; 

TextureCube < float4 > shaderTexture3 [ ] : register ( t0 , space2 ) ; 

SamplerState gsamPointWrap : register ( s0 ) ; 
SamplerState gsamPointClamp : register ( s1 ) ; 
SamplerState gsamLinearWrap : register ( s2 ) ; 
SamplerState gsamLinearClamp : register ( s3 ) ; 
SamplerState gsamAnisotoropicWrap : register ( s4 ) ; 
SamplerState gsamAnisotoropicClamp : register ( s5 ) ; 
SamplerComparisonState gsamAnisotoropicCompClamp : register ( s6 ) ; 
SamplerComparisonState gsamLinerCompClamp : register ( s7 ) ; 


























cbuffer CameraData : register ( b0 ) 
{ 
    float4x4 view ; 
    float4x4 projection ; 
    float4x4 viewProjection ; 
    
    float4x4 inverseView ; 
    float4x4 inverseProjection ; 
    float4x4 inverseViewProjection ; 
    
    float3 cameraWorldPosition ; 
    float1 pad0 ; 
    float3 look ; 
    float1 pad1 ; 
} ; 


















float4x4 GetWorldMatrix ( uint instanceID , VertexData vertex ) 
{ 
    return objectInfo [ instanceID ] . world ; 
} 







SurfaceInput VertexShaderFunction ( VertexData input , uint InstanceID : SV_InstanceID ) 
{ 
    SurfaceInput result ; 
    float4x4 worldMatrix = GetWorldMatrix ( InstanceID , input ) ; 
    result . worldPosition = mul ( float4 ( input . position , 1 ) , worldMatrix ) ; 
    result . position = mul ( result . worldPosition , viewProjection ) ; 
    result . worldNormal = mul ( float4 ( input . normal , 0 ) , worldMatrix ) ; 
    result . worldTangent = mul ( float4 ( input . tangent , 0 ) , worldMatrix ) ; 
    result . worldBiTangent = mul ( float4 ( input . bitangent , 0 ) , worldMatrix ) ; 
    result . uv = input . uv ; 
    result . InstanceID = InstanceID ; 
    
    return result ; 
} 
































struct ShadowPSInput 
{ 
    float4 position : SV_Position ; 
} ; 

float4 PixelShaderFunction ( ShadowPSInput outStream ) : SV_TARGET 
{ 
    return float4 ( 1 , 1 , 1 , 1 ) ; 
} 









struct MaterialData 
{ 
    uint ColorTextureIndex ; 
    uint NormalTextureIndex ; 
    uint MetalicTextureIndex ; 
    uint RoughnessTextureIndex ; 
    float SpecularValue ; 
    float2 UVSize ; 
    float pad ; 
} ; 


StructuredBuffer < MaterialData > materialData : register ( t1 ) ; 


Surface UserSurfaceFunction ( SurfaceInput input ) 
{ 
    Surface result ; 
    MaterialData mat = materialData [ objectInfo [ input . InstanceID ] . materialIndex ] ; 
    
    float2x2 uvScale = float2x2 ( mat . UVSize . x , 0 , 0 , mat . UVSize . y ) ; 
    float2 uv = mul ( input . uv , uvScale ) ; 
    
    result . albedo = shaderTexture [ mat . ColorTextureIndex ] . Sample ( gsamAnisotoropicWrap , uv ) . xyz ; 
    result . reflection = objectInfo [ input . InstanceID ] . environmentMapIndex / 12000.0f ; 
    
    result . specular = 0.0f ; 
    result . metalic = shaderTexture [ mat . MetalicTextureIndex ] . Sample ( gsamAnisotoropicWrap , uv ) . xxx ; 
    result . roughness = shaderTexture [ mat . RoughnessTextureIndex ] . Sample ( gsamAnisotoropicWrap , uv ) . xxx ; 
    result . emssion = 0.0f ; 
    
    float3x3 TBN = float3x3 ( 
    normalize ( input . worldTangent . xyz ) , 
    normalize ( input . worldBiTangent . xyz ) , 
    normalize ( input . worldNormal . xyz ) 
    ) ; 
    
    float3 normalMap = shaderTexture [ mat . NormalTextureIndex ] . Sample ( gsamAnisotoropicWrap , uv ) . xyz ; 
    normalMap = normalMap * 2 - 1.0f ; 
    result . wNormal = normalize ( mul ( normalMap , TBN ) ) ; 
    
    result . environmentMap = objectInfo [ input . InstanceID ] . environmentMapIndex ; 
    result . reserve0 = 1.0f . xxx ; 
    
    return result ; 
}  