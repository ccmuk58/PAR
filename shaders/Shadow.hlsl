#include "Common.hlsli"

float4 VSMain(float3 position : POSITION) : SV_Position
{
    float4 worldPos = mul(float4(position, 1.0), gWorld);
    return mul(worldPos, gLightViewProj);
}
