struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return input.WorldPosition;
}