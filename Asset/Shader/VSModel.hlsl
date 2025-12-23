cbuffer ViewProjection : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix VPMatrix;
}

cbuffer World : register(b1)
{
    matrix WorldMatrix;
    matrix NormalMatrix; // 스케일 역행렬을 적용한 월드 행렬
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
    float4 WorldPosition : POSITION0;
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3x3 TBN : TBN0;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    output.WorldPosition = mul(input.Position, WorldMatrix);
    output.Position = mul(output.WorldPosition, VPMatrix);
    
    output.UV = input.UV;
    
    output.TBN[2] = normalize(mul(float4(input.Normal, 0.0f), NormalMatrix).xyz); // 이거 normalize 꼭 필요하나?
    output.TBN[0] = normalize(mul(float4(input.Tangent, 0.0f), NormalMatrix).xyz);
    output.TBN[1] = cross(output.TBN[2], output.TBN[0]);
    
    
    return output;
}