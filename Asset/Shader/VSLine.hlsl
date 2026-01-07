/// VSLine.hlsl의 시작
#include "CommonVS.hlsli"

VS_OUTPUT_POS main(VS_INPUT_POS input)
{
    VS_OUTPUT_POS output;

    float4 world_position = mul(input.Position, WorldMatrix);
    output.Position = mul(world_position, VPMatrix);    
        
    return output;
}
/// VSLine.hlsl의 끝