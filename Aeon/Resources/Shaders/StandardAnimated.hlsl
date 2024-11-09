#stage vertex
#include "Include/CameraBuffer.hlsli"

struct VertexInput
{
    float3 pos          : POSITION;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float2 uv           : UV;
	float3 color        : COLOR;
	uint4 boneIDs       : BONEIDS;
	float4 boneWeights  : BONEWEIGHTS;
};

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    float4 worldPos : WORLDPOSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv       : UV;
    float3 color    : COLOR;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 OB_Transform;
}

cbuffer BoneBuffer : register(b4)
{
    float4x4 BB_BoneTransforms[128];
}

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    const float3x3 rotationMatrix = (float3x3)OB_Transform;
    
    float3 normal = normalize(mul(rotationMatrix, input.normal));
    float3 tangent = normalize(mul(rotationMatrix, input.tangent));
    float3 binormal = cross(normal, tangent);
    
    const float4x4 skin = 
        mul(input.boneWeights.x, BB_BoneTransforms[input.boneIDs.x]) +
        mul(input.boneWeights.y, BB_BoneTransforms[input.boneIDs.y]) +
        mul(input.boneWeights.z, BB_BoneTransforms[input.boneIDs.z]) +
        mul(input.boneWeights.w, BB_BoneTransforms[input.boneIDs.w]);
        
    normal      = normalize(mul(skin, float4(normal, 0.0f))).rgb;
    tangent     = normalize(mul(skin, float4(tangent, 0.0f))).rgb;
    binormal    = normalize(mul(skin, float4(binormal, 0.0f))).rgb;
    
    float4 position = mul(skin, float4(input.pos, 1.0f));
    
    output.worldPos = mul(OB_Transform, position);
    output.pos = mul(CB_ViewProj, output.worldPos);
    output.normal = normal;
    output.tangent = tangent;
    output.binormal = binormal;
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

#stage pixel
#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/Lighting.hlsli"
#include "Include/LightBuffer.hlsli"
#include "Include/CameraBuffer.hlsli"

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    float4 worldPos : WORLDPOSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv       : UV;
    float3 color    : COLOR;
};

Texture2D albedoTexture         : register(t0);
Texture2D normalTexture         : register(t1);
Texture2D materialTexture       : register(t2);
Texture2D brdfLUT               : register(t3);
 
TextureCube environmentTexture  : register(t4);

float4 main(VertexOutput input) : SV_TARGET
{
    const float3x3 tbn = float3x3
    (
        input.tangent,
        input.binormal,
        input.normal
    );
    
    const float3 albedoTexColor = ToLinear(albedoTexture.Sample(wrapSampler, input.uv).rgb);
    
    float3 pixelNormal = normalTexture.Sample(wrapSampler, input.uv).xyz;
    pixelNormal = 2 * pixelNormal - 1;
    pixelNormal.z = sqrt(1 - (pixelNormal.x * pixelNormal.x) + (pixelNormal.y * pixelNormal.y));
    pixelNormal = normalize(mul(normalize(pixelNormal), tbn));
    
    const float3 materialValues = materialTexture.Sample(wrapSampler, input.uv).rgb;
    
    const float occlusion = materialValues.r;
    const float roughness = materialValues.g;
    const float metalness = materialValues.b;
    
    const float3 specularColor = lerp((float3) 0.04f, albedoTexColor, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedoTexColor, 1.0f - metalness);
    
    
    const float3 V = normalize(CB_CameraPos - input.worldPos.xyz);
    
    const int numMips = max(GetNumMips(environmentTexture) - 1, 0);
    const float3 iblDiffuse = environmentTexture.SampleLevel(clampSampler, pixelNormal, numMips).rgb;
    
    const float3 R = reflect(-V, pixelNormal);
    const float3 envColor = environmentTexture.SampleLevel(clampSampler, R, roughness * numMips).rgb;
    
    const float nDotV = saturate(dot(pixelNormal, V));
    const float2 brdf = brdfLUT.Sample(brdfLUTSampler, float2(nDotV, roughness)).rg;
    const float3 iblSpecular = envColor * (specularColor * brdf.x + brdf.y);
    
    const float3 ambience = (diffuseColor * iblDiffuse + iblSpecular) * occlusion;
    
    
    const float3 directLightContribution = CalculateLight(CB_CameraPos, input.worldPos.xyz, pixelNormal, LB_Direction, diffuseColor, specularColor, LB_Color.rgb, LB_Intensity, roughness);
    
    
    float3 result = ambience * LB_EnvironmentIntensity + directLightContribution;
    result = result / (result + 0.155f) * 1.019f;
    return float4(ToGamma(result), 1.0f);
}
