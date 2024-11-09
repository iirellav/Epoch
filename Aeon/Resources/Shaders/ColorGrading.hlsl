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

Texture2D sourceTexture : register(t0);
Texture2D lutTextue : register(t1);

#define MAXCOLOR 15.0
#define COLORS 16.0
#define WIDTH 256.0
#define HEIGHT 16.0

float4 main(VertexOutput input) : SV_TARGET
{
    const float4 sourceCol = sourceTexture.Sample(brdfLUTSampler, input.uv);

    float cell = sourceCol.b * MAXCOLOR;

    float cellL = floor(cell);
    float cellH = ceil(cell);

    float halfSourceX = 0.5f / WIDTH;
    float halfSourceY = 0.5f / HEIGHT;
    float rOffset = halfSourceX + sourceCol.r / COLORS * (MAXCOLOR / COLORS);
    float gOffset = halfSourceY + sourceCol.g * (MAXCOLOR / COLORS);

    float2 lutPosL = float2(cellL / COLORS + rOffset, gOffset);
    float2 lutPosH = float2(cellH / COLORS + rOffset, gOffset);

    float4 lutColorL = lutTextue.Sample(brdfLUTSampler, lutPosL);
    float4 lutColorH = lutTextue.Sample(brdfLUTSampler, lutPosH);
    
    float4 gradedColor = lerp(lutColorL, lutColorH, frac(cell));

    return float4(gradedColor.rgb, 1.0f);
}
