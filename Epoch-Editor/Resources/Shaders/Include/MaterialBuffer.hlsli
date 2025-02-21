cbuffer MaterialBuffer : register(b1)
{
    float3 MB_AlbedoColor;
    float MB_NormalStrength;

    float2 MB_UVTiling;
    float2 MB_UVOffset;
    
    float3 MB_EmissionColor;
    float MB_EmissionStrength;
    
    float MB_Roughness;
    float MB_Metalness;
}