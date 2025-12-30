/// PSSkybox.hlsl의 시작
#include "CommonPS.hlsli"
//TextureCube environmentMap : register(t1);
//SamplerState samplerState : register(s1);

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 ViewDir : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    return environmentMapTexture.Sample(SamplerLinearWrap, normalize(input.ViewDir));
    //return environmentMap.Sample(samplerState, normalize(input.ViewDir));
}
/// PSSkybox.hlsl의 끝