cbuffer CameraBuffer : register(b0)
{
    float4x4 CB_ViewProj;
    float3 CB_CameraPos;
}