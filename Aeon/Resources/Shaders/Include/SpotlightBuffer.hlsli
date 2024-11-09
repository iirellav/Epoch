cbuffer SpotlightBuffer : register(b2)
{
    float4x4 SLB_ViewProj;
    
    float3 SLB_Position;
    float SLB_Intensity;
    
    float3 SLB_Direction;
    float SLB_Range;
    
    float3 SLB_Color;
    float SLB_ConeAngle;
    
    float SLB_ConeAngleDiff;
}