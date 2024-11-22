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

struct VertexOutput
{
    float4 pos  : SV_POSITION;
    float2 uv   : UV;
};

const float Posterize(const float value, const int steps)
{
    return round(value * steps) / steps;
}

SamplerState clampSampler : register(s1);
Texture2D sceneTexture : register(t4);

float4 main(VertexOutput input) : SV_TARGET
{
    const float4 orgColor = sceneTexture.Sample(clampSampler, input.uv);
    float3 HSV = RGBToHSV(orgColor.rgb);
    
    HSV.z = Posterize(HSV.z, 8);
    
    return float4(HSVToRGB(HSV), 1.0f);
}
