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

cbuffer PostProcessingBuffer : register(b0)
{
    float3 Vignette_Color;
    float Vignette_Size;
    float2 Vignette_Center;
    float Vignette_Intensity;
    float Vignette_Smoothness;
    
    uint Tonemap;
    uint Flags;
};

float3 Tonemap_UnrealEngine(float3 input)
{
    // Unreal 3, Documentation: "Color Grading"
    // Adapted to be close to Tonemap_ACES, with similar range
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return input / (input + 0.155) * 1.019;
}

float3 Tonemap_Lottes(float3 input)
{
    // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
    const float a = 1.6;
    const float d = 0.977;
    const float hdrMax = 8.0;
    const float midIn = 0.18;
    const float midOut = 0.267;
    
    // Can be precomputed
    const float b =
        (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const float c =
        (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    
    return pow(input, a) / (pow(input, a * d) * b + c);
}

float3 Tonemap_ACES(float3 input)
{
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (input * (a * input + b)) / (input * (c * input + d) + e);
}

float4 Tonemapp(const float4 color)
{
    float3 output = color.rgb;
    
    switch (Tonemap)
    {
        case 1:
            output = Tonemap_UnrealEngine(output);
            break;
        case 2:
            output = ToGamma(Tonemap_Lottes(output));
            break;
        case 3:
            output = ToGamma(Tonemap_ACES(output));
            break;
        default:
            output = ToGamma(output);
            break;
    }
    
    return float4(output, 1.0f);
}

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

    float4 lutColorL = lutTextue.SampleLevel(LUTSampler, lutPosL, 0);
    float4 lutColorH = lutTextue.SampleLevel(LUTSampler, lutPosH, 0);
    
    float4 gradedColor = lerp(lutColorL, lutColorH, frac(cell));

    return float4(gradedColor.rgb, 1.0f);
}

float4 Vignette(const float4 color, const float2 uv)
{
    float2 pos = uv - 0.5f;
    pos *= Vignette_Size;
    pos += 0.5f;
    
    float2 d = abs(pos - Vignette_Center) * Vignette_Intensity;
    //d = pow(saturate(d), _Roundness);
    float vfactor = pow(saturate(1.0f - dot(d, d)), Vignette_Smoothness);

    return float4(lerp(Vignette_Color, color.rgb, vfactor), 1.0f);
}

float4 main(VertexOutput input) : SV_TARGET
{
    float4 sourceCol = sourceTexture.SampleLevel(LUTSampler, input.uv, 0);
    
    bool colorGradingEnabled =  (Flags >> 0) & 1;
    bool vignetteEnabled =      (Flags >> 1) & 1;
    
    sourceCol = saturate(Tonemapp(sourceCol));
    if (colorGradingEnabled)
    {
        sourceCol = ColorGrade(sourceCol);
    }
    
    if (vignetteEnabled)
    {
        sourceCol = Vignette(sourceCol, input.uv);
    }
    
    return float4(sourceCol.rgb, 1.0f);
}
