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
#include "Include/GBuffer.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/Lighting.hlsli"
#include "Include/SpotlightBuffer.hlsli"
#include "Include/CameraBuffer.hlsli"

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
};

Texture2D cookieTexture : register(t5);

float4 main(VertexOutput input) : SV_TARGET
{
    const float3 albedo = albedoTexture.Sample(clampSampler, input.uv).rgb;
    const float3 material = materialTexture.Sample(clampSampler, input.uv).rgb;
    const float3 normal = DecodeOct(normalTexture.Sample(clampSampler, input.uv).rg);
    
    const float depth = depthTexture.Sample(clampSampler, input.uv).x;
    float3 worldPos = ClipToWorldSpace(input.uv, depth, CB_InvViewProj);
    
    const float occlusion = material.r;
    const float roughness = material.g;
    const float metalness = material.b;
    
    const float3 specularColor = lerp((float3) 0.04f, albedo.rgb, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedo.rgb, 1.0f - metalness);
    
    const float d = distance(SLB_Position, worldPos);
    if (d > SLB_Range)
    {
        discard;
        return 0.0f.xxxx;
    }
    
    const float3 l = normalize(worldPos - SLB_Position);
    
    const float3 lightContribution = CalculateLight(CB_CameraPos, worldPos, normal, l, diffuseColor, specularColor, SLB_Color, SLB_Intensity, roughness);
    
    const float coneAttenuation = pow(saturate(saturate(dot(SLB_Direction, l) - SLB_ConeAngle) / max(SLB_ConeAngleDiff, 0.0001f)), 2.0f);
    const float rangeAttenuation = saturate(1.0f - pow(d * (1.0f / SLB_Range), 2.0f));
    //const float normalAttenuation = saturate(dot(-l, normal));
    
    const float4 lightSpacePos = mul(SLB_ViewProj, float4(worldPos, 1.0f));
    float2 lightSpaceUV = lightSpacePos.xy / lightSpacePos.w * 0.5 + 0.5;
    lightSpaceUV.y = 1 - lightSpaceUV.y;
    
    float3 cookie = cookieTexture.Sample(clampSampler, lightSpaceUV).rgb;
    float cookieIntensity = RGBToHSV(cookie).b;
    
    return float4(lightContribution * coneAttenuation * rangeAttenuation /* * normalAttenuation*/ * cookie, 1.0f);
}
