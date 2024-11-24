#stage vertex

struct VertexInput
{
    float3 pos      : POSITION;
    uint texIndex   : TEXINDEX;
    float4 tint     : TINT;
    float2 uv       : UV;
    uint entityID   : ID;
};

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    uint texIndex   : TEXINDEX;
    float4 tint     : TINT;
    float2 uv       : UV;
    uint entityID   : ID;
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
    output.texIndex = input.texIndex;
    output.entityID = input.entityID;
    return output;
}

#stage pixel

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    uint texIndex   : TEXINDEX;
    float4 tint     : TINT;
    float2 uv       : UV;
    uint entityID   : ID;
};

SamplerState clampSampler : register(s1);
Texture2D textures[32] : register(t0);

struct Output
{
    float4 color : SV_TARGET0;
    uint entityID : SV_TARGET1;
};

Output main(VertexOutput input) : SV_TARGET
{
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    switch (input.texIndex)
    {
        case  0: texColor = textures[ 0].Sample(clampSampler, input.uv); break;
        case  1: texColor = textures[ 1].Sample(clampSampler, input.uv); break;
        case  2: texColor = textures[ 2].Sample(clampSampler, input.uv); break;
        case  3: texColor = textures[ 3].Sample(clampSampler, input.uv); break;
        case  4: texColor = textures[ 4].Sample(clampSampler, input.uv); break;
        case  5: texColor = textures[ 5].Sample(clampSampler, input.uv); break;
        case  6: texColor = textures[ 6].Sample(clampSampler, input.uv); break;
        case  7: texColor = textures[ 7].Sample(clampSampler, input.uv); break;
        case  8: texColor = textures[ 8].Sample(clampSampler, input.uv); break;
        case  9: texColor = textures[ 9].Sample(clampSampler, input.uv); break;
        case 10: texColor = textures[10].Sample(clampSampler, input.uv); break;
        case 11: texColor = textures[11].Sample(clampSampler, input.uv); break;
        case 12: texColor = textures[12].Sample(clampSampler, input.uv); break;
        case 13: texColor = textures[13].Sample(clampSampler, input.uv); break;
        case 14: texColor = textures[14].Sample(clampSampler, input.uv); break;
        case 15: texColor = textures[15].Sample(clampSampler, input.uv); break;
        case 16: texColor = textures[16].Sample(clampSampler, input.uv); break;
        case 17: texColor = textures[17].Sample(clampSampler, input.uv); break;
        case 18: texColor = textures[18].Sample(clampSampler, input.uv); break;
        case 19: texColor = textures[19].Sample(clampSampler, input.uv); break;
        case 20: texColor = textures[20].Sample(clampSampler, input.uv); break;
        case 21: texColor = textures[21].Sample(clampSampler, input.uv); break;
        case 22: texColor = textures[22].Sample(clampSampler, input.uv); break;
        case 23: texColor = textures[23].Sample(clampSampler, input.uv); break;
        case 24: texColor = textures[24].Sample(clampSampler, input.uv); break;
        case 25: texColor = textures[25].Sample(clampSampler, input.uv); break;
        case 26: texColor = textures[26].Sample(clampSampler, input.uv); break;
        case 27: texColor = textures[27].Sample(clampSampler, input.uv); break;
        case 28: texColor = textures[28].Sample(clampSampler, input.uv); break;
        case 29: texColor = textures[29].Sample(clampSampler, input.uv); break;
        case 30: texColor = textures[30].Sample(clampSampler, input.uv); break;
        case 31: texColor = textures[31].Sample(clampSampler, input.uv); break;
        default: texColor = float4(1.0f, 1.0f, 1.0f, 1.0f); break;
    }
    
    Output output;
    
    if (texColor.a * input.tint.a < 0.000001f)
    {
        discard;
        output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        output.entityID = 0;
        return output;
    }
    output.color = texColor * input.tint;
    output.entityID = input.entityID;
    return output;
}
