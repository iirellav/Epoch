#stage vertex

struct VertexOutput
{
    float4 pos  : SV_POSITION;
    float2 uv   : UV;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 CB_InvViewProj;
}

cbuffer GridBuffer : register(b1)
{
    float4x4 GB_Transform;
    float2 GB_Size;
    float GB_Alpha;
}

VertexOutput main(unsigned int aVertexIndex : SV_VertexID)
{
    const float4 pos[6] =
    {
        float4(-1, 0, 1, 1),
        float4(1, 0, 1, 1),
        float4(1, 0, -1, 1),
        float4(-1, 0, 1, 1),
        float4(1, 0, -1, 1),
        float4(-1, 0, -1, 1)
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
    
    const float3 position = pos[aVertexIndex].rgb * 50.f * float3(GB_Size.x, 1.0f, GB_Size.y);
    output.pos = mul(CB_InvViewProj, mul(GB_Transform, float4(position, 1.0f)));
    output.uv = uv[aVertexIndex];
    
    return output;
}

#stage pixel

struct VertexOutput
{
    float4 pos  : SV_POSITION;
    float2 uv   : UV;
};

cbuffer GridBuffer : register(b1)
{
    float4x4 GB_Transform;
    float2 GB_Size;
    float GB_Alpha;
}

const float grid(const float2 scaledUV, const float size)
{
    const float2 grid = frac(scaledUV);
    return step(size, grid.x) * step(size, grid.y);
}

#define LineWidth 0.025f

float4 main(VertexOutput input) : SV_TARGET
{
    const float x = grid(input.uv * (GB_Size + (float2) LineWidth), LineWidth);
    const float4 color = float4((float3)0.2f, GB_Alpha) * (1.0f - x);
	
    if (color.a < 0.0001f)
    {
        discard;
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    
    return color;
}
