/// VSLine.hlsl의 시작
#include "CommonVS.hlsli"

//// 모두 CommonVS.hlsli을 보냄
//

//cbuffer ViewProjection : register(b0)
//{
//    matrix ViewMatrix;
//    matrix ProjectionMatrix;
//    matrix VPMatrix;
//}

//cbuffer World : register(b1)
//{
//    matrix WorldMatrix;
//}

//struct VertexInput
//{
//    float4 Position : POSITION;
//};

//struct VertexOutput
//{
//    float4 Position : SV_POSITION;
//};

//VertexOutput main(VertexInput input)
VS_OUTPUT_POS main(VS_INPUT_POS input)
{
    VS_OUTPUT_POS output;

    float4 world_position = mul(input.Position, WorldMatrix);
    output.Position = mul(world_position, VPMatrix);    
        
    return output;
}
/// VSLine.hlsl의 끝