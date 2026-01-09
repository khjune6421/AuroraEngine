/// CommonPS.hlsli의 시작
#ifndef __COMMON_PS_HLSLI__
#define __COMMON_PS_HLSLI__

// --------------------------------------------------------
// Constant Buffers
// --------------------------------------------------------

cbuffer CameraPosition : register(b0)
{
    float4 CameraPosition;
};

cbuffer DirectionalLight : register(b1)
{
    float4 LightDirection;
    float4 LightColor;
};

cbuffer MaterialFactor : register(b2)
{
    float4 AlbedoFactor;
    
    float ambientOcclusionFactor;
    float RoughnessFactor;
    float MetallicFactor;
    float iorFactor;
    
    float4 EmissionFactor;
};

// --------------------------------------------------------
// Sampler States
// --------------------------------------------------------

SamplerState SamplerPointClamp : register(s0); // PostProcess
SamplerState SamplerLinearWrap : register(s1); // Model, Skybox


// --------------------------------------------------------
// Textures
// --------------------------------------------------------
Texture2D sceneTexture : register(t0);
TextureCube environmentMapTexture : register(t1); // 나중에 반사광에 사용?

// PBR 재질
Texture2D albedoTexture : register(t2);
Texture2D ORMTexture : register(t3); // ambient occlusion(R) + roughness(G) + metallic(B)
Texture2D normalTexture : register(t4);

// --------------------------------------------------------
// Input Structures (VS 출력과 매칭되어야 함)
// --------------------------------------------------------

struct PS_INPUT_STD
{
    float4 WorldPosition : POSITION0;
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3x3 TBN : TBN0;
};

struct PS_INPUT_POS_UV
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

struct PS_INPUT_POS
{
    float4 Position : SV_POSITION;
};


#endif // __COMMON_PS_HLSLI__
/// CommonPS.hlsli의 끝