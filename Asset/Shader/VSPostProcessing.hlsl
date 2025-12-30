/// VSModel.hlsl의 시작
#include "CommonVS.hlsli"

//// 모두 CommonVS.hlsli을 보냄
//
//struct VertexInput
//{
//    float4 Position : POSITION;
//    float2 UV : TEXCOORD;
//};

//struct VertexOutput
//{
//    float4 Position : SV_POSITION;
//    float2 UV : TEXCOORD;
//};

VS_OUTPUT_POS_UV main(VS_INPUT_POS_UV input)
{
    return input;
}