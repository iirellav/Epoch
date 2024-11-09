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

struct VertexOutput
{
    float4 pos  : SV_POSITION;
    float2 uv   : UV;
};

const float3 RGBToHSV(const float3 RGB)
{
    const float4 K = float4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
    const float4 p = lerp(float4(RGB.bg, K.wz), float4(RGB.gb, K.xy), step(RGB.b, RGB.g));
    const float4 q = lerp(float4(p.xyw, RGB.r), float4(RGB.r, p.yzx), step(p.x, RGB.r));
    
    const float d = q.x - min(q.w, q.y);
    const float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0f * d + e)), d / (q.x + e), q.x);
}

const float3 HSVToRGB(const float3 HSV)
{
    const float4 K = float4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    const float3 p = abs(frac(HSV.xxx + K.xyz) * 6.0f - K.www);
    return HSV.z * lerp(K.xxx, clamp(p - K.xxx, 0.0f, 1.0f), HSV.y);
}

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
