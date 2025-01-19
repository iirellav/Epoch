#stage vertex
#include "Include/CameraBuffer.hlsli"

struct VertexInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : UV;
    float3 color : COLOR;
    float4 row[3] : ROW;
    uint entityID : ID;
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
    uint entityID : ID;
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
    output.entityID = input.entityID;
    
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
    uint entityID : ID;
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

struct GBufferOutput
{
    float4 albedo : SV_TARGET0;
    float4 material : SV_TARGET1;
    float4 normals : SV_TARGET2;
    float4 worldPos : SV_TARGET3;
    uint entityID : SV_TARGET4;
};

SamplerState wrapSampler : register(s0);

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D materialTexture : register(t2);

GBufferOutput main(VertexOutput input)
{
    GBufferOutput output;
    
    const float3x3 tbn = float3x3
    (
        input.tangent,
        input.binormal,
        input.normal
    );
    
    const float2 scaledUV = input.uv * MB_UVTiling;
    
    const float3 albedoColor = ToLinear(albedoTexture.Sample(wrapSampler, scaledUV).rgb) * MB_AlbedoColor;
    
    float3 pixelNormal = normalTexture.Sample(wrapSampler, scaledUV).xyz;
    pixelNormal = 2 * pixelNormal - 1;
    pixelNormal.z = sqrt(1 - (pixelNormal.x * pixelNormal.x) + (pixelNormal.y * pixelNormal.y));
    pixelNormal.xy *= MB_NormalStrength;
    pixelNormal = normalize(mul(normalize(pixelNormal), tbn));
    
    const float4 materialValues = materialTexture.Sample(wrapSampler, scaledUV);
    
    output.albedo = float4(albedoColor, 1.0f) + float4(MB_EmissionColor * MB_EmissionStrength * materialValues.a, 1.0f);
    output.material = materialValues * float4(1.0f, MB_Roughness, MB_Metalness, 1.0f);
    output.normals = float4(EncodeOct(pixelNormal), 0.0f, 1.0f);
    output.worldPos = input.worldPos;
    output.entityID = input.entityID + 1;
    
    return output;
}
