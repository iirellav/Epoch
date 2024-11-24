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

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

/* For 2D
float ScreenPxRange()
{
	float pixRange = 2.0f;
	float geoSize = 72.0f;
	return geoSize / 32.0f * pixRange;
}
*/

SamplerState clampSampler : register(s1);
Texture2D fontAtlases[32] : register(t0);

struct Output
{
    float4 color : SV_TARGET0;
    uint entityID : SV_TARGET1;
};

Output main(VertexOutput input)
{
    float4 fgColor = input.tint;
    float4 msd = float4(1.0f, 1.0f, 1.0f, 1.0f);
    switch (input.texIndex)
    {
        case  0: msd = fontAtlases[ 0].Sample(clampSampler, input.uv); break;
        case  1: msd = fontAtlases[ 1].Sample(clampSampler, input.uv); break;
        case  2: msd = fontAtlases[ 2].Sample(clampSampler, input.uv); break;
        case  3: msd = fontAtlases[ 3].Sample(clampSampler, input.uv); break;
        case  4: msd = fontAtlases[ 4].Sample(clampSampler, input.uv); break;
        case  5: msd = fontAtlases[ 5].Sample(clampSampler, input.uv); break;
        case  6: msd = fontAtlases[ 6].Sample(clampSampler, input.uv); break;
        case  7: msd = fontAtlases[ 7].Sample(clampSampler, input.uv); break;
        case  8: msd = fontAtlases[ 8].Sample(clampSampler, input.uv); break;
        case  9: msd = fontAtlases[ 9].Sample(clampSampler, input.uv); break;
        case 10: msd = fontAtlases[10].Sample(clampSampler, input.uv); break;
        case 11: msd = fontAtlases[11].Sample(clampSampler, input.uv); break;
        case 12: msd = fontAtlases[12].Sample(clampSampler, input.uv); break;
        case 13: msd = fontAtlases[13].Sample(clampSampler, input.uv); break;
        case 14: msd = fontAtlases[14].Sample(clampSampler, input.uv); break;
        case 15: msd = fontAtlases[15].Sample(clampSampler, input.uv); break;
        case 16: msd = fontAtlases[16].Sample(clampSampler, input.uv); break;
        case 17: msd = fontAtlases[17].Sample(clampSampler, input.uv); break;
        case 18: msd = fontAtlases[18].Sample(clampSampler, input.uv); break;
        case 19: msd = fontAtlases[19].Sample(clampSampler, input.uv); break;
        case 20: msd = fontAtlases[20].Sample(clampSampler, input.uv); break;
        case 21: msd = fontAtlases[21].Sample(clampSampler, input.uv); break;
        case 22: msd = fontAtlases[22].Sample(clampSampler, input.uv); break;
        case 23: msd = fontAtlases[23].Sample(clampSampler, input.uv); break;
        case 24: msd = fontAtlases[24].Sample(clampSampler, input.uv); break;
        case 25: msd = fontAtlases[25].Sample(clampSampler, input.uv); break;
        case 26: msd = fontAtlases[26].Sample(clampSampler, input.uv); break;
        case 27: msd = fontAtlases[27].Sample(clampSampler, input.uv); break;
        case 28: msd = fontAtlases[28].Sample(clampSampler, input.uv); break;
        case 29: msd = fontAtlases[29].Sample(clampSampler, input.uv); break;
        case 30: msd = fontAtlases[30].Sample(clampSampler, input.uv); break;
        case 31: msd = fontAtlases[31].Sample(clampSampler, input.uv); break;
        default: msd = float4(1.0f, 1.0f, 1.0f, 1.0f); break;
    }
    
    float sd = median(msd.r, msd.g, msd.b);
    
    uint mips = 0;
    uint elements = 0;
    uint2 dim = uint2(0, 0);
    switch (input.texIndex)
    {
        case  0: fontAtlases[ 0].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  1: fontAtlases[ 1].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  2: fontAtlases[ 2].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  3: fontAtlases[ 3].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  4: fontAtlases[ 4].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  5: fontAtlases[ 5].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  6: fontAtlases[ 6].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  7: fontAtlases[ 7].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  8: fontAtlases[ 8].GetDimensions(mips, dim.x, dim.y, elements); break;
        case  9: fontAtlases[ 9].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 10: fontAtlases[10].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 11: fontAtlases[11].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 12: fontAtlases[12].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 13: fontAtlases[13].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 14: fontAtlases[14].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 15: fontAtlases[15].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 16: fontAtlases[16].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 17: fontAtlases[17].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 18: fontAtlases[18].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 19: fontAtlases[19].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 20: fontAtlases[20].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 21: fontAtlases[21].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 22: fontAtlases[22].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 23: fontAtlases[23].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 24: fontAtlases[24].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 25: fontAtlases[25].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 26: fontAtlases[26].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 27: fontAtlases[27].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 28: fontAtlases[28].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 29: fontAtlases[29].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 30: fontAtlases[30].GetDimensions(mips, dim.x, dim.y, elements); break;
        case 31: fontAtlases[31].GetDimensions(mips, dim.x, dim.y, elements); break;
    }
    
    float pxRange = 2.0f;
    float2 unitRange = (float2) pxRange / float2(dim.x, dim.y);
    float2 screenTexSize = (float2) 1.0f / fwidth(input.uv);
    float screenPxDistance = max(0.5f * dot(unitRange, screenTexSize), 1.0f) * (sd - 0.5f);
    
    Output output;
    
    float opacity = clamp(screenPxDistance + 0.5f, 0.0f, 1.0f);
    if (opacity < 0.000001f)
    {
        discard;
        output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        output.entityID = 0;
        return output;
    }
    output.color = fgColor;
    output.entityID = input.entityID;
    return output;
}
