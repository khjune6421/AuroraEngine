struct VertexInput
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

VertexOutput main(VertexInput input)
{
    return input;
}