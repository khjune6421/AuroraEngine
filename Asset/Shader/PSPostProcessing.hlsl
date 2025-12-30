/// PSPostProcessing.hlsl의 시작
#include "CommonPS.hlsli"
//Texture2D sceneTexture : register(t0);
//SamplerState sceneSampler : register(s0);

//struct PixelInput
//{
//    float4 Position : SV_POSITION;
//    float2 UV : TEXCOORD;
//};

float4 main(PS_INPUT_POS_UV input) : SV_TARGET
{
    return sceneTexture.Sample(SamplerPointClamp, input.UV);
}

/// PSPostProcessing.hlsl의 끝