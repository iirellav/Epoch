#stage vertex

struct VertexOutput
{
    float4 pos  : SV_POSITION;
    float2 uv   : UV;
};

VertexOutput main(unsigned int aVertexIndex : SV_VertexID)
{
    const float4 pos[6] =
    {
        float4(-1, 1, 0, 1),
        float4(1, 1, 0, 1),
        float4(1, -1, 0, 1),
        float4(-1, 1, 0, 1),
        float4(1, -1, 0, 1),
        float4(-1, -1, 0, 1)
    };
    
    const float2 uv[6] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
    };
    
    VertexOutput output;
    
    output.pos = pos[aVertexIndex];
    output.uv = uv[aVertexIndex];
    
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
    float4 pos  : SV_POSITION;
    float2 uv   : UV;
};

Texture2D albedoTexture : register(t0);
Texture2D materialTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D emissionTexture : register(t3);
Texture2D worldPositionTexture : register(t4);
TextureCube environmentTexture : register(t10);
Texture2D brdfLUT : register(t11);

float3 main(VertexOutput input) : SV_TARGET
{
    const float3 albedo = albedoTexture.Sample(clampSampler, input.uv).rgb;
    const float3 material = materialTexture.Sample(clampSampler, input.uv).rgb;
    const float3 normal = DecodeOct(normalTexture.Sample(clampSampler, input.uv).rg);
    const float3 worldPos = worldPositionTexture.Sample(clampSampler, input.uv).rgb;
    
    const float occlusion = material.r;
    const float roughness = material.g;
    const float metalness = material.b;
    
    const float3 specularColor = lerp((float3) 0.04f, albedo, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedo, 1.0f - metalness);
    
    const float3 V = normalize(CB_CameraPos - worldPos);
    
    const int numMips = max(GetNumMips(environmentTexture) - 1, 0);
    const float3 iblDiffuse = environmentTexture.SampleLevel(clampSampler, normal, numMips).rgb;
    
    const float3 R = reflect(-V, normal);
    const float3 envColor = environmentTexture.SampleLevel(clampSampler, R, roughness * numMips).rgb;
    
    const float nDotV = saturate(dot(normal, V));
    const float2 brdf = brdfLUT.Sample(LUTSampler, float2(nDotV, roughness)).rg;
    const float3 iblSpecular = envColor * (specularColor * brdf.x + brdf.y);
    
    const float3 ambience = (diffuseColor * iblDiffuse + iblSpecular) * occlusion;
    
    const float3 directLightContribution = CalculateLight(CB_CameraPos, worldPos, normal, LB_Direction, diffuseColor, specularColor, LB_Color.rgb, LB_Intensity, roughness);
    
    const float3 result = ambience * LB_EnvironmentIntensity + directLightContribution;
    return float4(result, 1.0f);
}
