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
SamplerState samplerState : register(s0);
Texture2D sourceTexture : register(t0);

float4 main(VertexOutput input) : SV_TARGET
{
    return float4(sourceTexture.SampleLevel(samplerState, input.uv, 0));
}
