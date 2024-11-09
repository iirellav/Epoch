#stage compute

#define pi 3.14159265359f

static const uint NumSamples = 1024;
static const float InvNumSamples = 1.0f / float(NumSamples);

cbuffer MipFilterBuffer : register(b0)
{
    float FB_Roughness;
}

// Function to compute the radical inverse of a value (Van Der Corput sequence)
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Function to sample Hammersley points
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) * InvNumSamples, RadicalInverse_VdC(i));
}

// Texture and sampler bindings
TextureCube<float4> input : register(t0);
RWTexture2DArray<float4> output : register(u0);

SamplerState samplerState : register(s1);

// Helper function to sample GGX normal distribution
float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
    float a = Roughness * Roughness;
    float phi = 2.0 * pi * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    // Transform H to world space
    float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
    float3 sampleVec = H.x * tangent + H.y * bitangent + H.z * N;
    return normalize(sampleVec);
}

float3 GetCubeMapTexCoord(float2 uv, uint face)
{
    float3 ret;
    if      (face == 0)   ret = float3(1.0f, uv.y, -uv.x);     // +X
    else if (face == 1)   ret = float3(-1.0f, uv.y, uv.x);     // -X
    else if (face == 2)   ret = float3(uv.x, -1.0f, uv.y);     // -Y
    else if (face == 3)   ret = float3(uv.x, 1.0f, -uv.y);     // +Y
    else if (face == 4)   ret = float3(uv.x, uv.y, 1.0f);      // +Z
    else if (face == 5)   ret = float3(-uv.x, uv.y, -1.0f);    // -Z
    return normalize(ret);
}

// Thread group size
[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // Ensure we are within output bounds
    uint2 outputSize;
    uint elements;
    output.GetDimensions(outputSize.x, outputSize.y, elements);
    if (DTid.x >= outputSize.x || DTid.y >= outputSize.y) return;

    // Get the normalized direction for this texel
    const float2 uv = (DTid.xy / float2(outputSize)) * 2.0f - 1.0f;
    float3 N = GetCubeMapTexCoord(uv, DTid.z);
    float3 V = N;
    float3 PrefilteredColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;

    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, FB_Roughness, N);
        float3 L = 2.0f * dot(V, H) * H - V;

        float NoL = saturate(dot(N, L));
        if (NoL > 0.0f)
        {
            // Sample from the texture
            float3 flippedL = float3(L.x, -L.y, L.z);
            PrefilteredColor += input.SampleLevel(samplerState, flippedL, 0).rgb * NoL;
            TotalWeight += NoL;
        }
    }

    PrefilteredColor /= TotalWeight;
    output[DTid] = float4(PrefilteredColor, 1.0f);
}
