cbuffer CameraPosition : register(b0)
{
    float4 cameraPosition;
};

cbuffer DirectionalLight : register(b1)
{
    float4 lightDirection; // 정규화된 방향 벡터여야 함
    float4 lightColor;
};

cbuffer MaterialFactor : register(b2)
{
    float4 albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float ambientOcclusionFactor;
    float lightFactor;
    float4 emissionFactor;
};

SamplerState textureSampler : register(s1);

TextureCube environmentMapTexture : register(t1); // 나중에 반사광에 사용?

Texture2D albedoTexture : register(t2);
Texture2D normalTexture : register(t3);
Texture2D metallicTexture : register(t4);
Texture2D roughnessTexture : register(t5);
Texture2D ambientOcclusionTexture : register(t6);

struct PixelInput
{
    float4 WorldPosition : POSITION0;
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3x3 TBN : TBN0;
};

float4 main(PixelInput input) : SV_TARGET
{
    float4 albedo = albedoTexture.Sample(textureSampler, input.UV) * albedoFactor;
    float3 normal = mul(normalTexture.Sample(textureSampler, input.UV).xyz * 2.0f - 1.0f, input.TBN);
    
    albedo.rgb *= lightColor.rgb * dot(normal, lightDirection.xyz) * lightFactor;
    
    return albedo + emissionFactor;
}