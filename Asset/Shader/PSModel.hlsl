/// PSModel.hlsl의 시작
#include "CommonPS.hlsli"

float4 main(PS_INPUT_STD input) : SV_TARGET
{
    float4 albedo = albedoTexture.Sample(SamplerLinearWrap, input.UV) * AlbedoFactor;
    float3 normal = mul(normalTexture.Sample(SamplerLinearWrap, input.UV).xyz * 2.0f - 1.0f, input.TBN);
    
    albedo.rgb *= LightColor.rgb * dot(normal, LightDirection.xyz);
    albedo.rgb += LightColor.rgb * LightColor.w; // 앰비언트 광원
    
    return albedo + EmissionFactor;
}
/// PSModel.hlsl의 끝