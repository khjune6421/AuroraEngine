cbuffer ViewProjection : register(b0)
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix VPMatrix;
}

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
    
    // 회전만 반영된 뷰 행렬
    matrix viewNoTranslation = ViewMatrix;
    viewNoTranslation[3] = float4(0, 0, 0, 1);
    
    // 역투영행렬 * 역뷰행렬
    matrix invVP = transpose(mul(viewNoTranslation, ProjectionMatrix));
    
    // 원점에서 멀리 떨어진 위치 계산
    output.ViewDir = mul(output.Position, invVP).xyz;
    
    return output;
}