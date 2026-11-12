struct VSInput
{
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

cbuffer PreFrame : register(b0)
{
    float4x4 gWorldViewProj;

};


PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0), gWorldViewProj);
    output.color    = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return float4(input.color, 1.0);
}
