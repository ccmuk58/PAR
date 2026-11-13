struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
};

cbuffer PerFrame : register(b0)
{
    float4x4 gWorldViewProj;

};


PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0), gWorldViewProj);
    output.normal   = input.normal;
    output.uv       = input.uv;
    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return float4(normalize(input.normal) * 0.5 + 0.5, 1.0);
}
