/// VSModel.hlsl의 시작
#include "CommonVS.hlsli"

// 범용적이지 않아도 될것 같음
struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 ViewDir : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;
    
    // 전체 화면을 덮는 2개의 삼각형 정점 위치 계산
    float2 texCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.Position = float4(texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 1.0f, 1.0f);
    
    // 원점에서 멀리 떨어진 위치 계산
    output.ViewDir = mul(output.Position, SkyboxVPMatrix).xyz;
    
    return output;
}