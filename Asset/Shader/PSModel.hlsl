#include "CommonPS.hlsli"
#include "CommonMath.hlsli"

float4 main(PS_INPUT_STD input) : SV_TARGET
{
    float4 albedo = albedoTexture.Sample(SamplerLinearWrap, input.UV) * AlbedoFactor;
    float3 orm = ORMTexture.Sample(SamplerLinearWrap, input.UV).xyz * float3(AmbientOcclusionFactor, RoughnessFactor, MetallicFactor);
    float4 bump = normalTexture.Sample(SamplerLinearWrap, input.UV);
    
    float3 N = UnpackNormal(bump.rgb, input.TBN, NormalScale);
    float3 L = -LightDirection.xyz; // 라이트 벡터
    float NdotL = saturate(dot(N, L));
    
    float3 V = normalize(CameraPosition.xyz - input.WorldPosition.xyz); // 뷰 벡터
    float3 H = normalize(V + L); // 반사 벡터
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.rgb, orm.b); // 금속도에 따른 F0 계산
    
    float NDF = DistributionGGX(N, H, orm.g); // 분포 함수
    float G = GeometrySmith(N, V, L, orm.g); // 지오메트리 함수
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0); // 프레넬 효과
    
    float3 numerator = NDF * G * F; // 분자
    float denominator = 4.0 * saturate(dot(N, V)) * NdotL + 0.0001; // 분모
    float3 specular = numerator / denominator; // 스페큘러 반사
    
    float3 kD = (float3(1.0, 1.0, 1.0) - F) * (1.0 - orm.z); // 디퓨즈 반사
    
    float3 radiance = LightColor.rgb * NdotL; // 조명 세기
    
    float3 ambient = LightColor.rgb * albedo.xyz * orm.r * radiance; // 앰비언트 조명
    float3 Lo = (kD * albedo.rgb / PI + specular) * radiance; // 최종 조명 계산
    
    albedo.xyz = ambient + Lo;
    
    return albedo + EmissionFactor;
}