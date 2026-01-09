#include "CommonPS.hlsli"
#include "CommonMath.hlsli"

float4 main(PS_INPUT_STD input) : SV_TARGET
{
    float4 albedo = albedoTexture.Sample(SamplerLinearWrap, input.UV) * AlbedoFactor;
    float3 normal = normalize(mul(UnpackNormal(normalTexture.Sample(SamplerLinearWrap, input.UV).rgb), input.TBN));
    
    // 조명 계산
    float NdotL = saturate(dot(normal, -LightDirection.xyz));
    float3 ambient = albedo.rgb * LightColor.xyz * LightColor.w; // 앰비언트 조명
    float3 diffuse = NdotL * LightColor.xyz * LightDirection.w; // 디퓨즈 조명
    
    albedo.rgb *= ambient + diffuse;
    
    return albedo + EmissionFactor;
}