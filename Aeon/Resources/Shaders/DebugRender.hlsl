#stage vertex

struct VertexInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : UV;
    float3 color : COLOR;
    float4 row[3] : ROW;
};

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : UV;
    float3 color : COLOR;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 CB_ViewProj;
    float3 CB_CameraPos;
}

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

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : UV;
    float3 color : COLOR;
};

cbuffer MaterialBuffer : register(b1)
{
    float3 MB_AlbedoColor;
    float MB_NormalStrength;

    float2 MB_UVTiling;
    float MB_Roughness;
    float MB_Metalness;
    
    float3 MB_EmissionColor;
    float MB_EmissionStrength;
}

cbuffer DrawModeBuffer : register(b2)
{
    uint DB_DrawMode;
}

SamplerState wrapSampler : register(s0);

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D materialTexture : register(t2);

float4 main(VertexOutput input) : SV_TARGET
{
    const float3x3 tbn = float3x3
    (
        input.tangent,
        input.binormal,
        input.normal
    );
    
    const float2 scaledUV = input.uv * MB_UVTiling;
    
    const float3 albedoColor = albedoTexture.Sample(wrapSampler, scaledUV).rgb * MB_AlbedoColor;
    
    float3 pixelNormal = normalTexture.Sample(wrapSampler, scaledUV).xyz;
    pixelNormal = 2 * pixelNormal - 1;
    pixelNormal.z = sqrt(1 - (pixelNormal.x * pixelNormal.x) + (pixelNormal.y * pixelNormal.y));
    pixelNormal.xy *= MB_NormalStrength;
    pixelNormal = normalize(mul(normalize(pixelNormal), tbn));
    
    const float3 materialValues = materialTexture.Sample(wrapSampler, scaledUV).rgb * float3(1.0f, MB_Roughness, MB_Metalness);
    
    switch (DB_DrawMode)
    {
        case 1: return float4(albedoColor, 1.0f); //Albedo
        case 2: return float4((pixelNormal + 1.0f) * 0.5f, 1.0f); //Normals
        case 3: return float4(materialValues.rrr, 1.0f); //AmbientOcclusion
        case 4: return float4(materialValues.ggg, 1.0f); //Roughness
        case 5: return float4(materialValues.bbb, 1.0f); //Metalness
        case 6: return float4(input.worldPos.xyz, 1.0f); //WorldPosition
    }
    
    //output.albedoNormX = float4(albedoColor, normals.x);
    //output.materialNormY = float4(materialValues * float3(1.0f, MB_Roughness, MB_Metalness), normals.y);
    //output.emission = float4(MB_EmissionColor * MB_EmissionStrength, 1.0f);
    //output.worldPos = input.worldPos;
    
    return float4(0.0f.rrrr);
}
