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

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
};

cbuffer VignetteBuffer : register(b0)
{
    float3 CB_Color;
    float CB_Size;
    float2 CB_Center;
    float CB_Intensity;
    float CB_Smoothness;
};

Texture2D sourceTexture : register(t0);

float4 main(VertexOutput input) : SV_TARGET
{
    const float4 sourceCol = sourceTexture.Sample(brdfLUTSampler, input.uv);
    
    float2 pos = input.uv - 0.5f;
    pos *= CB_Size;
    pos += 0.5f;
    
    float2 d = abs(pos - CB_Center) * CB_Intensity;
    //d = pow(saturate(d), _Roundness);
    float vfactor = pow(saturate(1.0f - dot(d, d)), CB_Smoothness);

    return float4(lerp(CB_Color, sourceCol.rgb, vfactor), 1.0f);
}
