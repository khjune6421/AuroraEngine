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
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    input.Position.w = 1.0f;
    output.Position = mul(input.Position, WVP);
    
    output.UV = input.UV;
    
    return output;
}