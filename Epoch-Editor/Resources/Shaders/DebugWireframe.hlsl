#stage vertex

struct VertexInput
{
    float3 pos          : POSITION;
	float4 color        : COLOR;
};

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    float4 color    : COLOR;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 CB_InvViewProj;
}

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.pos = mul(CB_InvViewProj, float4(input.pos, 1.0f));
    output.color = input.color;
    return output;
}

#stage pixel

struct VertexOutput
{
    float4 pos      : SV_POSITION;
    float4 color    : COLOR;
};

cbuffer LineBuffer : register(b0)
{
    float4 LB_Tint;
}

float4 main(VertexOutput input) : SV_TARGET
{
    if (input.color.a <= 0.0001f)
    {
        discard;
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    
    return input.color * LB_Tint;
}
