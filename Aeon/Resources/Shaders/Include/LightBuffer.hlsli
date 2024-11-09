cbuffer LightBuffer : register(b2)
{
    float3 LB_Direction;
    float LB_Intensity;
    float3 LB_Color;
    float LB_EnvironmentIntensity;
}