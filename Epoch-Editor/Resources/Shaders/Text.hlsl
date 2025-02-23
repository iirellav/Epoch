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
Texture2D fontAtlas : register(t0);

struct Output
{
    float4 color : SV_TARGET0;
    uint entityID : SV_TARGET1;
};

Output main(VertexOutput input)
{
    Output output;
    
    float4 fgColor = input.tint;
    float4 msd = fontAtlas.Sample(clampSampler, input.uv);
    
    float sd = median(msd.r, msd.g, msd.b);
    
    uint mips = 0;
    uint elements = 0;
    uint2 dim = uint2(0, 0);
    fontAtlas.GetDimensions(mips, dim.x, dim.y, elements);
    
    float pxRange = 2.0f;
    float2 unitRange = (float2) pxRange / float2(dim.x, dim.y);
    float2 screenTexSize = (float2) 1.0f / fwidth(input.uv);
    float screenPxDistance = max(0.5f * dot(unitRange, screenTexSize), 1.0f) * (sd - 0.5f);
    
    float opacity = clamp(screenPxDistance + 0.5f, 0.0f, 1.0f);
    if (opacity < 0.000001f)
    {
        discard;
        output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        output.entityID = 0;
        return output;
    }
    
    output.color = fgColor;
    output.entityID = input.entityID + 1;
    return output;
}
