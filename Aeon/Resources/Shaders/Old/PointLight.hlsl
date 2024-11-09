#stage vertex

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
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
#include "Include/PointLightBuffer.hlsli"
#include "Include/CameraBuffer.hlsli"

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
};

Texture2D albedoNormXTexture : register(t0);
Texture2D materialNormYTexture : register(t1);
Texture2D emissionTexture : register(t2);
Texture2D worldPositionTexture : register(t3);

//Texture2D albedoTexture : register(t0);
//Texture2D normalTexture : register(t1);
//Texture2D materialTexture : register(t2);
//Texture2D emissionTexture : register(t3);
//Texture2D worldPositionTexture : register(t4);

float3 main(VertexOutput input) : SV_TARGET
{
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    const float4 albedo = albedoNormXTexture.Sample(clampSampler, input.uv);
    const float4 material = materialNormYTexture.Sample(clampSampler, input.uv);
    const float3 worldPos = worldPositionTexture.Sample(clampSampler, input.uv).rgb;
    
    const float3 normal = DecodeOct(float2(albedo.a, material.a));
    
    //const float3 albedo = albedoTexture.Sample(clampSampler, input.uv).rgb;
    //const float3 normal = normalTexture.Sample(clampSampler, input.uv).rgb;
    //const float3 material = materialTexture.Sample(clampSampler, input.uv).rgb;
    //const float3 emission = emissionTexture.Sample(clampSampler, input.uv).rgb;
    //const float3 worldPos = worldPositionTexture.Sample(clampSampler, input.uv).rgb;
    
    const float occlusion = material.r;
    const float roughness = material.g;
    const float metalness = material.b;
    
    const float3 specularColor = lerp((float3) 0.04f, albedo.rgb, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedo.rgb, 1.0f - metalness);
    
    const float d = distance(PLB_Position, worldPos);
    if (d > PLB_Range)
    {
        discard;
        return 0.0f.xxxx;
    }
    
    const float3 l = normalize(worldPos - PLB_Position);
    
    const float3 lightContribution = CalculateLight(CB_CameraPos, worldPos, normal, l, diffuseColor, specularColor, PLB_Color, PLB_Intensity, roughness);
    
    const float rangeAttenuation = saturate(1.0f - pow(d * (1.0f / max(PLB_Range, 0.001f)), 2.0f));
    const float normalAttenuation = saturate(dot(-l, normal));
    
    return float4(lightContribution * rangeAttenuation * normalAttenuation, 1.0f);
}
