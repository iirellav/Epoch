#stage vertex

struct VertexInput
{
    float3 pos      : POSITION;
    uint entityID   : ID;
    float4 tint     : TINT;
    float2 uv       : UV;
};

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    uint entityID   : ID;
    float4 tint     : TINT;
    float2 uv       : UV;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 CB_InvViewProj;
    float3 CB_CameraPos;
}

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.pos = mul(CB_InvViewProj, float4(input.pos, 1.0f));
    output.uv = input.uv;
    output.tint = input.tint;
    output.entityID = input.entityID;
    return output;
}

#stage pixel

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    uint entityID   : ID;
    float4 tint     : TINT;
    float2 uv       : UV;
};

SamplerState clampSampler : register(s1);
Texture2D sourceTexture : register(t0);

struct Output
{
    float4 color : SV_TARGET0;
    uint entityID : SV_TARGET1;
};

Output main(VertexOutput input)
{
    Output output;
    
    float4 texColor = sourceTexture.Sample(clampSampler, input.uv);
    
    if (texColor.a * input.tint.a < 0.1f)
    {
        discard;
        output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        output.entityID = 0;
        return output;
    }
    
    output.color = texColor * input.tint;
    output.entityID = input.entityID + 1;
    return output;
}
