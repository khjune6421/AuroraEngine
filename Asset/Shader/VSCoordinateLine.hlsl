#include "CommonVS.hlsli"

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
};

VS_OUTPUT main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VS_OUTPUT output;
    
    
    if (instanceID < 101) // X방향 선
    {
        output.WorldPosition = float4(vertexID * 100.0f - 50.0f, 0.0f, float(instanceID) - 50.0f, 1.0f);
    }
    else if (instanceID < 202) // Z방향 선
    {
        output.WorldPosition = float4(float(instanceID) - 151.0f, 0.0f, vertexID * 100.0f - 50.0f, 1.0f);
    }
    else // Y축 선
    {
        output.WorldPosition = float4(0.0f, vertexID * 100.0f - 50.0f, 0.0f, 1.0f);
    }
    
    output.Position = mul(output.WorldPosition, VPMatrix);
    output.WorldPosition.xz = abs(output.WorldPosition.xz * 0.05f);
    
    return output;
}