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

Texture2D sourceTexture : register(t0);
Texture2D lutTextue : register(t1);

struct VertexOutput
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
};

#define MAXCOLOR 15.0
#define COLORS 16.0
#define WIDTH 256.0
#define HEIGHT 16.0

cbuffer VignetteBuffer : register(b0)
{
    float3 VB_Color;
    float VB_Size;
    float2 VB_Center;
    float VB_Intensity;
    float VB_Smoothness;
};

float4 ColorGrade(const float4 color)
{
    float cell = color.b * MAXCOLOR;

    float cellL = floor(cell);
    float cellH = ceil(cell);

    float halfSourceX = 0.5f / WIDTH;
    float halfSourceY = 0.5f / HEIGHT;
    float rOffset = halfSourceX + color.r / COLORS * (MAXCOLOR / COLORS);
    float gOffset = halfSourceY + color.g * (MAXCOLOR / COLORS);

    float2 lutPosL = float2(cellL / COLORS + rOffset, gOffset);
    float2 lutPosH = float2(cellH / COLORS + rOffset, gOffset);

    float4 lutColorL = lutTextue.SampleLevel(brdfLUTSampler, lutPosL, 0);
    float4 lutColorH = lutTextue.SampleLevel(brdfLUTSampler, lutPosH, 0);
    
    float4 gradedColor = lerp(lutColorL, lutColorH, frac(cell));

    return float4(gradedColor.rgb, 1.0f);
}

float4 Vignette(const float4 color, const float2 uv)
{
    float2 pos = uv - 0.5f;
    pos *= VB_Size;
    pos += 0.5f;
    
    float2 d = abs(pos - VB_Center) * VB_Intensity;
    //d = pow(saturate(d), _Roundness);
    float vfactor = pow(saturate(1.0f - dot(d, d)), VB_Smoothness);

    return float4(lerp(VB_Color, color.rgb, vfactor), 1.0f);
}

float4 main(VertexOutput input) : SV_TARGET
{
    float4 sourceCol = sourceTexture.SampleLevel(brdfLUTSampler, input.uv, 0);
    sourceCol = sourceCol / (sourceCol + 0.155f) * 1.019f;
    sourceCol.a = 1.0f;
    sourceCol = ColorGrade(sourceCol);
    sourceCol = Vignette(sourceCol, input.uv);
    
    return float4(sourceCol.rgb, 1.0f);
}
