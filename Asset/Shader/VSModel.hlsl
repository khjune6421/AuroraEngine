cbuffer ViewProjection : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
}

cbuffer World : register(b1)
{
    matrix WorldMatrix;
    matrix NormalMatrix; // 스케일 역행렬을 적용한 월드 행렬
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
    
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), NormalMatrix).xyz); // 이거 normalize 꼭 필요하나?
    output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), NormalMatrix).xyz);
    
    return output;
}