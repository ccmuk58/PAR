#include "Common.hlsli"

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

Texture2D              gAlbedoMap     : register(t0);
Texture2D              gShadowMap     : register(t1);
SamplerState           gSampler       : register(s0);
SamplerComparisonState gShadowSampler : register(s1);

static const float3 kSpecular = float3(0.5, 0.5, 0.5);
static const float  kShininess = 64.0;

float ShadowFactor(float3 worldPos)
{
    float4 lightPos = mul(float4(worldPos, 1.0), gLightViewProj);
    float3 proj     = lightPos.xyz / lightPos.w;
    float2 uv       = proj.xy * float2(0.5, -0.5) + 0.5;

    // 3x3 PCF
    float sum = 0.0;
    [unroll] for (int y = -1; y <= 1; ++y)
    {
        [unroll] for (int x = -1; x <= 1; ++x)
            sum += gShadowMap.SampleCmpLevelZero(gShadowSampler, uv, proj.z, int2(x, y));
    }
    return sum / 9.0;
}

float3 BlinnPhong(float3 N, float3 V, float3 L, float3 albedo)
{
    float3 H = normalize(L + V);
    float  NdotL    = saturate(dot(N, L));
    float3 diffuse  = albedo * NdotL;
    float3 specular = kSpecular * pow(saturate(dot(N, H)), kShininess) * (NdotL > 0.0 ? 1.0 : 0.0);
    return diffuse + specular;
}

float RangeAttenuation(float dist, float range)
{
    float ratio  = dist / max(range, 1e-4);
    float window = saturate(1.0 - ratio * ratio);
    return window * window;
}

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
    float3 V = normalize(gCameraPos - input.worldPos);

    float3 albedo = gAlbedoMap.Sample(gSampler, input.uv).rgb;
    float3 color  = gAmbientColor * albedo;

    // directional light
    float shadow = ShadowFactor(input.worldPos);
    color += BlinnPhong(N, V, normalize(-gLightDir), albedo) * gLightColor * shadow;

    // point light
    float3 toPoint   = gPointPos - input.worldPos;
    float  pointDist = length(toPoint);
    color += BlinnPhong(N, V, toPoint / max(pointDist, 1e-4), albedo) * gPointColor
           * RangeAttenuation(pointDist, gPointRange);

    // spot light
    float3 toSpot   = gSpotPos - input.worldPos;
    float  spotDist = length(toSpot);
    float3 spotL    = toSpot / max(spotDist, 1e-4);
    float  cone     = smoothstep(gSpotCosOuter, gSpotCosInner, dot(-spotL, gSpotDir));
    color += BlinnPhong(N, V, spotL, albedo) * gSpotColor
           * RangeAttenuation(spotDist, gSpotRange) * cone;

    return float4(color, 1.0);
}
