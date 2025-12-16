cbuffer ViewProjection : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}
cbuffer WorldWVP : register(b1)
{
    matrix WorldMatrix;
    matrix WVP;
}

struct VertexInput
{
    float4 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    input.Position.w = 1.0f;
    output.Position = mul(input.Position, WVP);
    output.Color = float4(input.UV, 0.0f, 1.0f);
    
    return output;
}