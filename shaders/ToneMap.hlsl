cbuffer ToneMap : register(b0)
{
    float  gExposure;
    int    gOperator;
    float2 gPad;
};

Texture2D gHdrColor : register(t0);

float3 Reinhard(float3 color)
{
    return color / (1.0 + color);
}

float3 AcesFilmic(float3 color)
{
    return saturate((color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14));
}

float4 VSMain(uint id : SV_VertexID) : SV_Position
{
    float2 uv = float2((id << 1) & 2, id & 2);
    return float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}

float4 PSMain(float4 position : SV_Position) : SV_Target
{
    float3 color = gHdrColor.Load(int3(position.xy, 0)).rgb * gExposure;

    if (gOperator == 1)
        color = Reinhard(color);
    else if (gOperator == 2)
        color = AcesFilmic(color);

    return float4(color, 1.0);
}
