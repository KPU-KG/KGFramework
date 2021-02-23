






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













struct LightData 
{ 
    float3 Strength ; 
    float FalloffStart ; 
    float3 Direction ; 
    float FalloffEnd ; 
    float3 Position ; 
    float SpotPower ; 
    float4 padding0 ; 
    float4 padding1 ; 
} ; 

StructuredBuffer < LightData > lightInfo : register ( t0 ) ; 

struct ShadowData 
{ 
    uint shadowMapIndex [ 4 ] ; 
    float4x4 shadowMatrix [ 4 ] ; 
} ; 
StructuredBuffer < ShadowData > shadowInfo : register ( t0 , space4 ) ; 

Texture2D InputGBuffer0 : register ( t2 ) ; 
Texture2D InputGBuffer1 : register ( t3 ) ; 
Texture2D InputGBuffer2 : register ( t4 ) ; 
Texture2D < uint4 > InputGBuffer3 : register ( t5 ) ; 
Texture2D InputGBuffer4 : register ( t6 ) ; 

float3 DepthToWorldPosition ( float depth , float2 projPosition , float4x4 inverseViewProj ) 
{ 
    float4 projPos = float4 ( projPosition . x , projPosition . y , depth , 1.0f ) ; 
    float4 worldPos = mul ( projPos , inverseViewProj ) ; 
    return worldPos . xyz / worldPos . w ; 
} 

float2 ProjPositionToUV ( float2 projPosition ) 
{ 
    projPosition . x += 1.0f ; 
    projPosition . y += 1.0f ; 
    projPosition . x *= 0.5f ; 
    projPosition . y *= 0.5f ; 
    projPosition . y = 1 - projPosition . y ; 
    return projPosition ; 
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















float CalcAttenuation ( float distance , float falloffStart , float falloffEnd ) 
{ 
    float d = pow ( distance , 2 ) ; 
    float r = pow ( falloffEnd , 2 ) ; 
    return pow ( saturate ( 1 - pow ( d / r , 2 ) ) , 2 ) ; 
} 


float ndfGGX ( float cosLh , float roughness ) 
{ 
    float alpha = roughness * roughness ; 
    float alphaSq = alpha * alpha ; 
    
    float denom = ( cosLh * cosLh ) * ( alphaSq - 1.0 ) + 1.0 ; 
    return alphaSq / ( 3.14159265359 * denom * denom ) ; 
} 


float gaSchlickG1 ( float cosTheta , float k ) 
{ 
    return cosTheta / ( cosTheta * ( 1.0 - k ) + k ) ; 
} 


float gaSchlickGGX ( float cosLi , float cosLo , float roughness ) 
{ 
    float r = roughness + 1.0 ; 
    float k = ( r * r ) / 8.0 ; 
    return gaSchlickG1 ( cosLi , k ) * gaSchlickG1 ( cosLo , k ) ; 
} 


float3 fresnelSchlick ( float3 F0 , float cosTheta ) 
{ 
    return F0 + ( 1.0 - F0 ) * pow ( 1.0 - cosTheta , 5.0 ) ; 
} 

float4 CustomLightCalculator ( LightData light , Surface info , float3 lightDir , float3 cameraDir , float atten ) 
{ 
    float3 L = - lightDir ; 
    float3 V = cameraDir ; 
    float3 N = info . wNormal ; 
    float3 H = normalize ( L + V ) ; 
    
    float NDotV = saturate ( dot ( N , V ) ) ; 
    float NDotL = saturate ( dot ( N , L ) ) ; 
    float NDotH = saturate ( dot ( N , H ) ) ; 
    float VDotH = saturate ( dot ( V , H ) ) ; 
    
    float Fdielectric = 0.04f ; 
    float3 F0 = lerp ( Fdielectric . xxx , info . albedo . xyz , info . metalic . xxx ) ; 
    

    float3 F = fresnelSchlick ( F0 , VDotH ) ; 
    float D = ndfGGX ( NDotH , info . roughness ) ; 
    float G = gaSchlickGGX ( NDotL , NDotV , info . roughness ) ; 
    
    float3 kd = lerp ( float3 ( 1 , 1 , 1 ) - F , float3 ( 0.0f , 0.0f , 0.0f ) , info . metalic . xxx ) ; 
    

    float3 diffuseBRDF = kd * info . albedo ; 
    

    float3 specularBRDF = ( F * D * G ) / max ( 0.00001f , 4.0f * NDotL * NDotV ) ; 
    
    return float4 ( ( diffuseBRDF + specularBRDF ) * NDotL * light . Strength * atten , 1.0f ) ; 
    

} 

float4 CustomAmbientLightCalculator ( LightData light , Surface info , float3 lightDir , float3 cameraDir , float atten ) 
{ 
    
    float3 L = - lightDir ; 
    float3 V = cameraDir ; 
    float3 N = info . wNormal ; 
    float3 H = normalize ( L + V ) ; 
    
    float NDotV = saturate ( dot ( N , V ) ) ; 
    float NDotL = saturate ( dot ( N , L ) ) ; 
    float NDotH = saturate ( dot ( N , H ) ) ; 
    float VDotH = saturate ( dot ( V , H ) ) ; 
    
    float Fdielectric = 0.04f ; 
    float3 F0 = lerp ( Fdielectric . xxx , info . albedo . xyz , info . metalic . xxx ) ; 
    
    float3 F = fresnelSchlick ( F0 , VDotH ) ; 
    float G = gaSchlickGGX ( NDotL , NDotV , info . roughness ) ; 
    
    float3 kd = lerp ( float3 ( 1 , 1 , 1 ) - F , float3 ( 0.0f , 0.0f , 0.0f ) , 1.0f . xxx - info . metalic . xxx ) ; 
    
    float3 reflec = reflect ( cameraDir , info . wNormal ) ; 
    float3 envPixel = shaderTextureCube [ info . environmentMap ] . Sample ( gsamAnisotoropicWrap , normalize ( reflec ) ) . rgb ; 
    return float4 ( F * envPixel * info . albedo , 1.0f ) ; 
} 







TextureCube < float > shadowCube [ ] : register ( t0 , space1 ) ; 

float PointShadowPCF ( float3 toPixel , LightData lightData , ShadowData shadowData , float cosTheta ) 
{ 
    if ( shadowData . shadowMapIndex [ 0 ] == 0 ) 
    { 
        return 1.0f ; 
    } 
    float3 toPixelAbs = abs ( toPixel ) ; 
    float z = max ( toPixelAbs . x , max ( toPixelAbs . y , toPixelAbs . z ) ) ; 
    float2 lightPerspectiveValue = float2 ( shadowData . shadowMatrix [ 0 ] . _m22 , shadowData . shadowMatrix [ 0 ] . _m32 ) ; 
    float depth = ( lightPerspectiveValue . x * z + lightPerspectiveValue . y ) / z ; 
    float bias = 0.005f * tan ( acos ( cosTheta ) ) ; 
    bias = clamp ( bias , 0.0003f , 0.01f ) ; 
    return shadowCube [ shadowData . shadowMapIndex [ 0 ] ] . SampleCmpLevelZero ( gsamAnisotoropicCompClamp , normalize ( toPixel ) , depth - bias ) ; 
} 

float3 SphericialCoordToVec ( float2 sphere ) 
{ 
    return float3 ( 
    sin ( sphere . x ) * cos ( sphere . y ) , 
    sin ( sphere . x ) * sin ( sphere . y ) , 
    cos ( sphere . x ) 
    ) ; 
} 

float2 VectorToSphericialCoord ( float3 vec ) 
{ 
    return float2 ( acos ( vec . z / length ( vec ) ) , atan2 ( vec . y , vec . x ) ) ; 
} 

float3 GetOffsetVector ( float3 vec , uint index ) 
{ 
    static float2 offset [ 9 ] = 
    { 
        float2 ( - 1 , + 1 ) , 
        float2 ( + 0 , + 1 ) , 
        float2 ( + 1 , + 1 ) , 
        float2 ( - 1 , + 0 ) , 
        float2 ( + 0 , + 0 ) , 
        float2 ( + 1 , + 0 ) , 
        float2 ( - 1 , - 1 ) , 
        float2 ( + 0 , - 1 ) , 
        float2 ( + 1 , - 1 ) , 
    } ; 
    static float rad = 0.0174533f * 0.1f ; 
    float len = length ( vec ) ; 
    return SphericialCoordToVec ( VectorToSphericialCoord ( vec ) + offset [ index ] * rad ) ; 
} 

float3 GetPoissonVector ( float3 vec , float ratio , uint index ) 
{ 
    static float2 poissonDisk [ 16 ] = 
    { 
        float2 ( - 0.94201624 , - 0.39906216 ) , 
        float2 ( 0.94558609 , - 0.76890725 ) , 
        float2 ( - 0.094184101 , - 0.92938870 ) , 
        float2 ( 0.34495938 , 0.29387760 ) , 
        
        float2 ( - 0.91588581 , 0.45771432 ) , 
        float2 ( - 0.81544232 , - 0.87912464 ) , 
        float2 ( - 0.38277543 , 0.27676845 ) , 
        float2 ( 0.97484398 , 0.75648379 ) , 
        
        float2 ( 0.44323325 , - 0.97511554 ) , 
        float2 ( 0.53742981 , - 0.47373420 ) , 
        float2 ( - 0.26496911 , - 0.41893023 ) , 
        float2 ( 0.79197514 , 0.19090188 ) , 
        
        float2 ( - 0.24188840 , 0.99706507 ) , 
        float2 ( - 0.81409955 , 0.91437590 ) , 
        float2 ( 0.19984126 , 0.78641367 ) , 
        float2 ( 0.14383161 , - 0.14100790 ) 
    } ; 
    static float rad = 0.0174533f * 0.1f ; 
    float len = length ( vec ) ; 
    return SphericialCoordToVec ( VectorToSphericialCoord ( vec ) + poissonDisk [ index ] * rad * ratio ) ; 
} 

float PointShadowPoissonPCF ( float3 toPixel , LightData lightData , ShadowData shadowData , float cosTheta ) 
{ 
    if ( shadowData . shadowMapIndex [ 0 ] == 0 ) 
    { 
        return 1.0f ; 
    } 
    float3 toPixelAbs = abs ( toPixel ) ; 
    float z = max ( toPixelAbs . x , max ( toPixelAbs . y , toPixelAbs . z ) ) ; 
    float2 lightPerspectiveValue = float2 ( shadowData . shadowMatrix [ 0 ] . _m22 , shadowData . shadowMatrix [ 0 ] . _m32 ) ; 
    float depth = ( lightPerspectiveValue . x * z + lightPerspectiveValue . y ) / z ; 
    float result = 0.0f ; 
    
    float bias = 0.005f * tan ( acos ( cosTheta ) ) ; 
    bias = clamp ( bias , 0.0f , 0.01f ) ; 
    
    for ( uint n = 0 ; n < 16 ; n ++ ) 
    { 
        result += shadowCube [ shadowData . shadowMapIndex [ 0 ] ] . SampleCmpLevelZero ( gsamLinerCompClamp , GetPoissonVector ( normalize ( toPixel ) , 1.0f , n ) , depth - bias ) ; 
    } 
    result /= 16.0f ; 
    return result ; 
} 

float PointShadowPCSS ( float3 toPixel , LightData lightData , ShadowData shadowData , float cosTheta ) 
{ 
    if ( shadowData . shadowMapIndex [ 0 ] == 0 ) 
    { 
        return 1.0f ; 
    } 
    float3 toPixelAbs = abs ( toPixel ) ; 
    float z = max ( toPixelAbs . x , max ( toPixelAbs . y , toPixelAbs . z ) ) ; 
    float2 lightPerspectiveValue = float2 ( shadowData . shadowMatrix [ 0 ] . _m22 , shadowData . shadowMatrix [ 0 ] . _m32 ) ; 
    float depth = ( lightPerspectiveValue . x * z + lightPerspectiveValue . y ) / z ; 
    
    float bias = 0.005f * tan ( acos ( cosTheta ) ) ; 
    bias = clamp ( bias , 0.0f , 0.01f ) ; 
    
    float avgBlockerDepth = 0.0f ; 
    float blockerCount = 0.0f ; 
    for ( uint i = 0 ; i < 9 ; i += 1 ) 
    { 
        float3 toOffsetPixel = GetOffsetVector ( normalize ( toPixel ) , i ) ; 
        uint state = 0 ; 
        float sampleDepth = shadowCube [ shadowData . shadowMapIndex [ 0 ] ] . Sample ( gsamPointClamp , toOffsetPixel ) ; 
        if ( sampleDepth < depth - bias ) 
        { 
            blockerCount += 1 ; 
            avgBlockerDepth += sampleDepth ; 
        } 
    } 
    
    if ( blockerCount == 0.0 ) 
    return 1.0 ; 
    else if ( blockerCount >= 9 ) 
    return 0.0 ; 
    
    avgBlockerDepth /= blockerCount ; 
    
    float lightSize = 20.0f ; 
    float ratio = ( abs ( depth - avgBlockerDepth ) * lightSize / avgBlockerDepth ) ; 
    
    float result = 0.0f ; 
    for ( uint n = 0 ; n < 16 ; n ++ ) 
    { 
        result += shadowCube [ shadowData . shadowMapIndex [ 0 ] ] . SampleCmpLevelZero ( gsamAnisotoropicCompClamp , GetPoissonVector ( normalize ( toPixel ) , 0.5f + pow ( ratio * 32 , 32 ) , n ) , 
        depth - bias ) ; 
    } 
    result /= 16 ; 
    return result ; 
} 


struct LightVertexOutput 
{ 
    float4 position : SV_Position ; 
    uint InstanceID : SV_InstanceID ; 
} ; 

struct LightHSConstantOutput 
{ 
    float edges [ 4 ] : SV_TessFactor ; 
    float Inside [ 2 ] : SV_InsideTessFactor ; 
} ; 

static const float3 HemilDir [ 2 ] = 
{ 
    float3 ( 1.0f , 1.0f , 1.0f ) , 
    float3 ( - 1.0f , 1.0f , - 1.0f ) 
} ; 

struct LightHSOutput 
{ 
    float3 HemilDir : POSITION ; 
    uint InstanceID : SV_InstanceID ; 
} ; 

struct LightPixelInput 
{ 
    float4 position : SV_Position ; 
    float4 projPosition : POSITION ; 
    uint InstanceID : SV_InstanceID ; 
} ; 


float4x4 GetLightMatrix ( LightData light ) 
{ 
    float s = light . FalloffEnd * 1.0f ; 
    float x = light . Position . x ; 
    float y = light . Position . y ; 
    float z = light . Position . z ; 
    return float4x4 
    ( 
    s , 0 , 0 , 0 , 
    0 , s , 0 , 0 , 
    0 , 0 , s , 0 , 
    x , y , z , 1 
    ) ; 
} 


LightVertexOutput VertexShaderFunction ( uint InstanceID : SV_InstanceID ) 
{ 
    LightVertexOutput result ; 
    result . position = float4 ( 0.0f , 0.0f , 0.0f , 1.0f ) ; 
    result . InstanceID = InstanceID ; 
    return result ; 
} 

LightHSConstantOutput ConstantHS ( ) 
{ 
    LightHSConstantOutput output ; 
    float tessFactor = 18.0f ; 
    output . edges [ 0 ] = output . edges [ 1 ] = output . edges [ 2 ] = output . edges [ 3 ] = tessFactor ; 
    output . Inside [ 0 ] = output . Inside [ 1 ] = tessFactor ; 
    return output ; 
} 

[ domain ( "quad" ) ] 
[ partitioning ( "integer" ) ] 
[ outputtopology ( "triangle_ccw" ) ] 
[ outputcontrolpoints ( 4 ) ] 
[ patchconstantfunc ( "ConstantHS" ) ] 
LightHSOutput HullShaderFunction ( InputPatch < LightVertexOutput , 1 > input , uint PatchID : SV_PrimitiveID ) 
{ 
    LightHSOutput output ; 
    output . HemilDir = HemilDir [ PatchID ] ; 
    output . InstanceID = input [ 0 ] . InstanceID ; 
    return output ; 
} 

[ domain ( "quad" ) ] 
LightPixelInput DomainShaderFunction ( LightHSConstantOutput constant , float2 uv : SV_DomainLocation , OutputPatch < LightHSOutput , 4 > quad ) 
{ 
    float2 posClipSpace = uv . xy * 2.0f - 1.0f ; 
    
    float2 posClipSpaceAbs = abs ( posClipSpace . xy ) ; 
    float maxLen = max ( posClipSpaceAbs . x , posClipSpaceAbs . y ) ; 
    float3 normDir = normalize ( float3 ( posClipSpace . xy , maxLen - 1.0f ) * quad [ 0 ] . HemilDir ) ; 
    
    float4 position = float4 ( normDir . xyz , 1.0f ) ; 
    LightPixelInput output ; 
    float4x4 LightProjection = GetLightMatrix ( lightInfo [ quad [ 0 ] . InstanceID ] ) ; 
    LightProjection = mul ( LightProjection , view ) ; 
    LightProjection = mul ( LightProjection , projection ) ; 
    
    output . position = mul ( position , LightProjection ) ; 
    output . projPosition = output . position ; 
    output . InstanceID = quad [ 0 ] . InstanceID ; 
    
    return output ; 
} 

float4 PixelShaderFunction ( LightPixelInput input ) : SV_Target0 
{ 
    return float4 ( 0 , 1 , 0 , 1 ) ; 
    
    input . projPosition . xy /= input . projPosition . w ; 
    float2 uv = ProjPositionToUV ( input . projPosition . xy ) ; 
    Surface pixelData = PixelDecode ( 
    InputGBuffer0 . Sample ( gsamPointWrap , uv ) , 
    InputGBuffer1 . Sample ( gsamPointWrap , uv ) , 
    InputGBuffer2 . Sample ( gsamPointWrap , uv ) , 
    InputGBuffer3 . Sample ( gsamPointWrap , uv ) 
    ) ; 
    float depth = InputGBuffer4 . Sample ( gsamPointWrap , uv ) . x ; 
    
    LightData lightData = lightInfo [ input . InstanceID ] ; 
    ShadowData shadowData = shadowInfo [ input . InstanceID ] ; 
    float3 calcWorldPosition = DepthToWorldPosition ( depth , input . projPosition . xy , mul ( inverseProjection , inverseView ) ) ; 
    

    float3 cameraDirection = calcWorldPosition - cameraWorldPosition ; 
    float3 lightDirection = calcWorldPosition - lightData . Position ; 
    
    float distance = length ( lightDirection ) ; 
    
    float atten = CalcAttenuation ( distance , lightData . FalloffStart , lightData . FalloffEnd ) ; 
    
    float shadowFactor = PointShadowPoissonPCF ( lightDirection , lightData , shadowData , dot ( normalize ( lightDirection ) , normalize ( pixelData . wNormal ) ) ) ; 
    
    return CustomLightCalculator ( lightData , pixelData , normalize ( lightDirection ) , normalize ( - cameraDirection ) , atten ) * shadowFactor ; 
} 
 