cbuffer MaterialFactor : register(b0)
{
    float4 albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float lightFactor;
    float ambient;
};

SamplerState textureSampler : register(s0);

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicTexture : register(t2);
Texture2D roughnessTexture : register(t3);

struct PixelInput
{
    float4 Position : SV_POSITION0;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

float4 main(PixelInput input) : SV_TARGET
{
    float4 albedo = albedoTexture.Sample(textureSampler, input.UV) * albedoFactor;
    
    return albedo;
}