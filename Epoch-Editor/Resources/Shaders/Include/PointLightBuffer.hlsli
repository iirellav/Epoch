cbuffer PointLightBuffer : register(b2)
{
    float4x4 PLB_ViewProj;
    
    float3 PLB_Position;
    float PLB_Intensity;
    
    float3 PLB_Color;
    float PLB_Range;
}