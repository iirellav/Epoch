#stage compute

#define pi 3.14159265359f

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

Texture2D equirectangularTexture : register(t0);
RWTexture2DArray<float4> cubemap : register(u0);

SamplerState samplerState : register(s1);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint elements = 0;
    uint2 dim = uint2(0, 0);
    cubemap.GetDimensions(dim.x, dim.y, elements);
    const float2 uv = (DTid.xy / float2(dim)) * 2.0f - 1.0f;

    // Calculate the direction vector for the current pixel
    const float3 dir = GetCubeMapTexCoord(uv, DTid.z);

    // Convert direction vector to equirectangular coordinates
    float2 equirectUV;
    equirectUV.x = atan2(dir.z, dir.x) / (2.0f * pi) + 0.5f;
    equirectUV.y = asin(dir.y) / pi + 0.5f;

    // Sample the equirectangular texture
    const float4 color = equirectangularTexture.SampleLevel(samplerState, equirectUV, 0);

    // Write the color to the cubemap face
    cubemap[DTid] = color;
}
