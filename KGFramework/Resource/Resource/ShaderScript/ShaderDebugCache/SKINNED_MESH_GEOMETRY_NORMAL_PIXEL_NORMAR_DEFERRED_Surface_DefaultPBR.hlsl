






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



















StructuredBuffer < float4x4 > boneOffsetInfo : register ( t0 , space3 ) ; 
StructuredBuffer < float4x4 > animationTransformInfo : register ( t1 , space3 ) ; 



float4x4 GetWorldMatrix ( uint instanceID , VertexData vertex ) 
{ 
    float4x4 animationMatrix = ( float4x4 ) 0.0f ; 
    for ( int i = 0 ; i < 4 ; i ++ ) 
    { 
        int animationIndex = ( 64 * instanceID ) + vertex . bone [ i ] ; 
        float4x4 bone = boneOffsetInfo [ vertex . bone [ i ] ] ; 
        float4x4 anim = animationTransformInfo [ animationIndex ] ; 
        float4x4 vTobone = mul ( bone , anim ) ; 
        animationMatrix += vertex . weight [ i ] * vTobone ; 
    } 
    return animationMatrix ; 
} 




SurfaceInput VertexShaderFuction ( VertexData input , uint InstanceID : SV_InstanceID ) 
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




































struct GBufferOut 
{ 
    float4 gbuffer0 : SV_Target0 ; 
    float4 gbuffer1 : SV_Target1 ; 
    float4 gbuffer2 : SV_Target2 ; 
    uint4 gbuffer3 : SV_Target3 ; 
} ; 

float3 SNORMTOUNORM ( float3 normal ) 
{ 
    normal *= 0.5f ; 
    normal += 0.5f ; 
    return normal ; 
} 

float3 UNORMTOSNORM ( float3 normal ) 
{ 
    normal -= 0.5f ; 
    normal *= 2.0f ; 
    return normal ; 
} 


float2 OctWrap ( float2 v ) 
{ 
    return ( 1.0 - abs ( v . yx ) ) * ( v . xy >= 0.0 ? 1.0 : - 1.0 ) ; 
} 

float2 EncodeNormal ( float3 n ) 
{ 
    n /= ( abs ( n . x ) + abs ( n . y ) + abs ( n . z ) ) ; 
    n . xy = n . z >= 0.0 ? n . xy : OctWrap ( n . xy ) ; 
    n . xy = n . xy * 0.5 + 0.5 ; 
    return n . xy ; 
} 

float3 DecodeNormal ( float2 f ) 
{ 
    f = f * 2.0 - 1.0 ; 
    

    float3 n = float3 ( f . x , f . y , 1.0 - abs ( f . x ) - abs ( f . y ) ) ; 
    float t = saturate ( - n . z ) ; 
    n . xy += n . xy >= 0.0 ? - t : t ; 
    return normalize ( n ) ; 
} 


GBufferOut PixelEncode ( Surface surface ) 
{ 
    GBufferOut result ; 
    result . gbuffer0 . xyz = surface . albedo ; 
    result . gbuffer0 . w = surface . reflection ; 
    
    result . gbuffer1 . x = surface . specular ; 
    result . gbuffer1 . y = surface . metalic ; 
    result . gbuffer1 . z = surface . roughness ; 
    result . gbuffer1 . w = surface . emssion ; 
    
    result . gbuffer2 . xy = EncodeNormal ( surface . wNormal ) ; 
    
    result . gbuffer3 . x = surface . environmentMap ; 
    result . gbuffer3 . yzw = surface . reserve0 ; 
    
    return result ; 
} 

Surface PixelDecode ( float4 gbuffer0 , float4 gbuffer1 , float4 gbuffer2 , uint4 gbuffer3 ) 
{ 
    Surface result ; 
    result . albedo = gbuffer0 . xyz ; 
    result . reflection = gbuffer0 . w ; 
    
    result . specular = gbuffer1 . x ; 
    result . metalic = gbuffer1 . y ; 
    result . roughness = gbuffer1 . z ; 
    result . emssion = gbuffer1 . w ; 
    

    result . wNormal = normalize ( DecodeNormal ( gbuffer2 . xy ) ) ; 
    
    result . environmentMap = gbuffer3 . x ; 
    result . reserve0 = gbuffer3 . yzw ; 
    
    return result ; 
} 




GBufferOut PixelShaderFuction ( SurfaceInput input ) 
{ 
    Surface surface = UserSurfaceFunction ( input ) ; 
    GBufferOut bufferResult = PixelEncode ( surface ) ; 
    return bufferResult ; 
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