int GetNumMips(const TextureCube cubeTex)
{
    int width = 0;
    int height = 0;
    int numMips = 0;
    cubeTex.GetDimensions(0, width, height, numMips);
    return numMips;
}

float3 ToLinear(const float3 sRGB)
{
    bool3 cutoff = sRGB < (float3) 0.04045f;
    float3 higher = pow((sRGB + (float3) 0.055f) / (float3) 1.055f, (float3) 2.2f);
    float3 lower = sRGB / (float3) 12.92f;
    
    return lerp(higher, lower, cutoff);
}

const float3 ToGamma(const float3 sRGB)
{
    return pow(sRGB, 1.0f / 2.2f);
}

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

float2 OctWrap(const float2 v)
{
    return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}
 
float2 EncodeOct(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}

float3 DecodeOct(float2 f)
{
    f = f * 2.0 - 1.0;
 
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0 ? -t : t;
    return normalize(n);
}
