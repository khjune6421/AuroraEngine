/// PSPostProcessing.hlsl의 시작
#include "CommonPS.hlsli"

float4 main(PS_INPUT_POS_UV input) : SV_TARGET
{
    return sceneTexture.Sample(SamplerPointClamp, input.UV);
}

/// PSPostProcessing.hlsl의 끝