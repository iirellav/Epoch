#define pi 3.14159265359f

const float CalculateD(const float3 aPixelNormal, const float aRoughness, const float3 aHalfAngle)
{
    const float roughSqrSqr = (aRoughness * aRoughness) * (aRoughness * aRoughness);
    const float nDotH = dot(aPixelNormal, aHalfAngle);
    const float nDotHSqr = saturate(nDotH * nDotH);
    //const float nDotHSqr = saturate(dot(aPixelNormal, aHalfAngle) * dot(aPixelNormal, aHalfAngle));
    return roughSqrSqr / (pi * pow((nDotHSqr * (roughSqrSqr - 1) + 1), 2));
}

const float3 CalculateF(const float3 aViewAngle, const float3 aHalfAngle, const float3 aSpecularColor)
{
    const float vDotH = saturate(dot(aViewAngle, aHalfAngle));
    const float p = ((-5.55473f * vDotH) - 6.98316f) * vDotH;
    return aSpecularColor + ((float3) 1.0f - aSpecularColor) * pow(2.0f, p);
}

const float CalculateG1(const float3 aPixelNormal, const float aK, const float3 aValue)
{
    const float nDotVal = saturate(dot(aPixelNormal, aValue));
    return nDotVal / (nDotVal * (1.0f - aK) + aK);
}

const float CalculateG(const float aRoughness, const float3 aPixelNormal, const float3 aViewAngle, const float3 aPixelToLightDir)
{
    const float k = ((aRoughness + 1.0f) * (aRoughness + 1.0f)) * 0.125f;
    const float g1L = CalculateG1(aPixelNormal, k, aPixelToLightDir);
    const float g1V = CalculateG1(aPixelNormal, k, aViewAngle);
    return g1L * g1V;
}

const float3 CalculateKs(const float D, const float3 F, const float G, const float3 aPixelNormal, const float3 aViewAngle, const float3 aPixelToLight)
{
    const float nDotL = saturate(dot(aPixelNormal, aPixelToLight));
    const float nDotV = saturate(dot(aPixelNormal, aViewAngle));
    return saturate((D * F * G) / (4.0f * nDotL * nDotV));
}

const float3 CalculateLight(const float3 cameraPos, const float3 pixelPosition, const float3 normal, const float3 lightDirection, const float3 diffuseColor, const float3 specularColor, const float3 lightColor, const float lightIntensity, const float roughness)
{
    const float3 V = normalize(cameraPos - pixelPosition);
    const float3 L = -lightDirection;
    const float3 H = normalize(L + V);
    
    const float D = CalculateD(normal, roughness, H);
    const float3 F = CalculateF(V, H, specularColor);
    const float G = CalculateG(roughness, normal, V, L);
    
    const float3 kS = specularColor * CalculateKs(D, F, G, normal, V, L);
    
    const float3 kD = diffuseColor / pi * saturate(1.0f - kS);
    const float3 lightColorIntensity = lightColor * lightIntensity;
    return (kD + kS) * lightColorIntensity * saturate(dot(normal, L));
}