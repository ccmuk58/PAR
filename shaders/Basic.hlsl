struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_Position;
    float3 worldPos : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
};

cbuffer PerFrame : register(b0)
{
    float4x4 gViewProj;
    float3   gCameraPos;
    float    gPad0;
    float3   gLightDir;
    float    gPad1;
    float3   gLightColor;
    float    gPad2;
    float3   gAmbientColor;
    float    gPad3;
};

cbuffer PerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
};

Texture2D    gAlbedoMap : register(t0);
SamplerState gSampler   : register(s0);

static const float3 kSpecular = float3(0.5, 0.5, 0.5);
static const float  kShininess = 64.0;


PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(float4(input.position, 1.0), gWorld);
    output.position = mul(worldPos, gViewProj);
    output.worldPos = worldPos.xyz;
    output.normal   = mul(input.normal, (float3x3)gWorldInvTranspose);
    output.uv       = input.uv;
    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(-gLightDir);
    float3 V = normalize(gCameraPos - input.worldPos);
    float3 H = normalize(L + V);

    float3 albedo = gAlbedoMap.Sample(gSampler, input.uv).rgb;

    // Blinn-Phong
    float  NdotL    = saturate(dot(N, L));
    float3 diffuse  = albedo * NdotL;
    float3 specular = kSpecular * pow(saturate(dot(N, H)), kShininess) * (NdotL > 0.0 ? 1.0 : 0.0);

    float3 color = gAmbientColor * albedo + (diffuse + specular) * gLightColor;
    return float4(color, 1.0);
}
