/// CommonVS.hlsli의 시작
#ifndef __COMMON_VS_HLSLI__
#define __COMMON_VS_HLSLI__

// --------------------------------------------------------
// Constant Buffers 
// --------------------------------------------------------
cbuffer ViewProjection : register(b0)
{
    matrix ViewMatrix; // 전치 안된 뷰 행렬
    matrix ProjectionMatrix; // 전치 안된 투영 행렬
    matrix VPMatrix; // 전치 된 뷰 행렬과 투영 행렬의 곱
}

cbuffer Skybox : register(b1)
{
    matrix SkyboxVPMatrix; // 뷰 행렬에서 위치 성분을 제거한 행렬과 투영 행렬의 곱의 역행렬
}

cbuffer World : register(b2)
{
    matrix WorldMatrix;
    matrix NormalMatrix; // 스케일 역행렬을 적용한 월드 행렬
}

// --------------------------------------------------------
// Input Structures
// --------------------------------------------------------

struct VS_INPUT_STD
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_INPUT_POS_UV
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VS_INPUT_POS
{
    float4 Position : POSITION;
};

// --------------------------------------------------------
// Output Structures
// --------------------------------------------------------

struct VS_OUTPUT_STD
{
    float4 WorldPosition : POSITION0;
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3x3 TBN : TBN0;
};

struct VS_OUTPUT_POS_UV
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

struct VS_OUTPUT_POS
{
    float4 Position : SV_POSITION;
};

#endif // __COMMON_VS_HLSLI__
/// CommonVS.hlsli의 끝