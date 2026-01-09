#include "CommonPS.hlsli"
#include "CommonMath.hlsli"

float4 main(PS_INPUT_STD input) : SV_TARGET
{
    float4 albedo = albedoTexture.Sample(SamplerLinearWrap, input.UV) * AlbedoFactor;
    float3 orm = ORMTexture.Sample(SamplerLinearWrap, input.UV).xyz * float3(AmbientOcclusionFactor, RoughnessFactor, MetallicFactor);
    float4 bump = normalTexture.Sample(SamplerLinearWrap, input.UV);
    
    // 조명 계산
    float3 normal = UnpackNormal(bump.rgb, input.TBN, NormalScale);
    float NdotL = saturate(dot(normal.xyz, -LightDirection.xyz));
    float3 ambient = albedo.rgb * LightColor.xyz * LightColor.w; // 앰비언트 조명
    float3 diffuse = NdotL * LightColor.xyz * LightDirection.w; // 디퓨즈 조명
    
    // 최종 색상 계산
    float3 light = albedo.rgb * (ambient + diffuse) * LightFactor;
    float3 glow = albedo.rgb * glowFactor;
    
    albedo.rgb = light + glow;
    
    return albedo + EmissionFactor;
}