struct PixelInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
};

float4 main(PixelInput input) : SV_TARGET
{
    return input.Color;
}