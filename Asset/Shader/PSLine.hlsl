/// PSLine.hlsl의 시작
#include "CommonPS.hlsli"

float4 main(PS_INPUT_POS input) : SV_TARGET
{   
    return LineColor;
}
/// PSLine.hlsl의 끝