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
#include "Include/Lighting.hlsli"

struct VertexOutput
{
    float4 pos  : SV_POSITION;
    float2 uv   : UV;
};

SamplerState clampSampler : register(s1);

Texture2D inputTexture : register(t0);

float4 main(VertexOutput input) : SV_TARGET
{
    float4 color = inputTexture.Sample(clampSampler, input.uv);
    
    //if (color.a < 0.05f)
    //{
    //    discard;
    //    return float4(0, 0, 0, 1.f);
    //}
    
    color = color / (color + 0.155f) * 1.019f;
    color.a = 1.0f;
    return color;
}
