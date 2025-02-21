#stage vertex
#include "Include/CameraBuffer.hlsli"

struct VertexInput
{
    float3 pos      : POSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float2 uv       : UV;
	float3 color    : COLOR;
    float4 row[3]	: ROW;
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

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    float4x4 transform;
    
    transform._11_21_31_41 = float4(input.row[0].xyz, 0.0f);
    transform._12_22_32_42 = float4(input.row[1].xyz, 0.0f);
    transform._13_23_33_43 = float4(input.row[2].xyz, 0.0f);
    transform._14_24_34_44 = float4(input.row[0].w, input.row[1].w, input.row[2].w, 1.0f);
    
    const float3x3 rotationMatrix = (float3x3) transform;
    
    output.worldPos = mul(transform, float4(input.pos, 1));
    output.pos = mul(CB_ViewProj, output.worldPos);
    output.normal = normalize(mul(rotationMatrix, input.normal));
    output.tangent = normalize(mul(rotationMatrix, input.tangent));
    output.binormal = normalize(cross(output.normal, output.tangent));
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

cbuffer MaterialBuffer : register(b1)
{
    float3 MB_AlbedoColor;
    float MB_NormalStrength;

    float2 MB_UVTiling;
    float MB_Roughness;
    float MB_Metalness;
}

Texture2D albedoTexture         : register(t0);
Texture2D normalTexture         : register(t1);
Texture2D materialTexture       : register(t2);
Texture2D brdfLUT               : register(t3);
 
TextureCube environmentTexture  : register(t4);

float4 main(VertexOutput input) : SV_TARGET
{
    //return float4(input.normal + 1 * 0.5f, 1);
    
    const float3x3 tbn = float3x3
    (
        input.tangent,
        input.binormal,
        input.normal
    );
    
    const float2 scaledUV = input.uv * MB_UVTiling;
    
    const float3 albedoTexColor = ToLinear(albedoTexture.Sample(wrapSampler, scaledUV).rgb);
    const float3 albedoColor = albedoTexColor * MB_AlbedoColor;
    
    float3 pixelNormal = normalTexture.Sample(wrapSampler, scaledUV).xyz;
    pixelNormal = 2 * pixelNormal - 1;
    pixelNormal.z = sqrt(1 - (pixelNormal.x * pixelNormal.x) + (pixelNormal.y * pixelNormal.y));
    pixelNormal.xy *= MB_NormalStrength;
    pixelNormal = normalize(mul(normalize(pixelNormal), tbn));
    
    const float3 materialValues = materialTexture.Sample(wrapSampler, scaledUV).rgb;
    
    const float occlusion = materialValues.r;
    const float roughness = materialValues.g * MB_Roughness;
    const float metalness = materialValues.b * MB_Metalness;
    
    const float3 specularColor = lerp((float3) 0.04f, albedoColor, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedoColor, 1.0f - metalness);
    
    
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
    //return float4(result, 1.0f);
    return float4(ToGamma(result), 1.0f);
}
