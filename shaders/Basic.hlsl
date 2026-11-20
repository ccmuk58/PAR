#include "Common.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;
    float2 uv       : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_Position;
    float3 worldPos : POSITION;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;
    float2 uv       : TEXCOORD;
};

Texture2D              gAlbedoMap     : register(t0);
Texture2D              gShadowMap     : register(t1);
Texture2D              gNormalMap     : register(t2);
Texture2D              gHeightMap     : register(t3);
SamplerState           gSampler       : register(s0);
SamplerComparisonState gShadowSampler : register(s1);

float2 ParallaxOcclusion(float2 uv, float3 Vts)
{
    const float minLayers = 8.0;
    const float maxLayers = 64.0;
    float numLayers = lerp(maxLayers, minLayers, saturate(Vts.z));
    float layerDepth = 1.0 / numLayers;
    
    float2 deltaUV = -Vts.xy / max(Vts.z, 0.05) * gHeightScale / numLayers;
    
    float2 dx = ddx(uv);
    float2 dy = ddy(uv);
    
    float2 curUV = uv;
    float curLayer = 0.0;
    float curMapDepth = 1.0 - gHeightMap.SampleGrad(gSampler, curUV, dx, dy).r;
    
    [loop]
    for (int i = 0; i < (int) maxLayers && curLayer < curMapDepth; ++i)
    {
        curUV += deltaUV;
        curLayer += layerDepth;
        curMapDepth = 1.0 - gHeightMap.SampleGrad(gSampler, curUV, dx, dy).r;
    }
    
    // 직전 레이어와 현재 레이어 사이 보간
    float2 prevUV = curUV - deltaUV;
    float after = curMapDepth - curLayer;
    float before = (1.0 - gHeightMap.SampleGrad(gSampler, prevUV, dx, dy).r) - (curLayer - layerDepth);
    float w = after / (after - before);
    return lerp(curUV, prevUV, w);
}

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
    float3 specular = gSpecular * pow(saturate(dot(N, H)), gShininess) * (NdotL > 0.0 ? 1.0 : 0.0);
    return diffuse + specular;
}

float3 SampleNormal(float2 uv, float3 T, float3 B, float3 N)
{
    float3 n = gNormalMap.Sample(gSampler, uv).xyz * 2.0 - 1.0;
    return normalize(n.x * T + n.y * B + n.z * N);
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
    
    float h = gHeightMap.SampleLevel(gSampler, input.uv, 3).r;
    // float edge = min(min(input.uv.x, 1 - input.uv.x), min(input.uv.y, 1 - input.uv.y));
    // float scale = gDisplacementScale * saturate(edge * 20.0);
    float3 pos = input.position + input.normal * (h - 1.0) * gDisplacementScale;
    float4 worldPos = mul(float4(pos, 1.0), gWorld);
    
    output.position = mul(worldPos, gViewProj);
    output.worldPos = worldPos.xyz;
    output.normal   = mul(input.normal, (float3x3)gWorldInvTranspose);
    output.tangent  = float4(mul(input.tangent.xyz, (float3x3)gWorld), input.tangent.w);
    output.uv       = input.uv;
    
    return output;
}


/*
struct PSInput
{
    float4 position : SV_Position;
    float3 worldPos : POSITION;
    float3 normal   : NORMAL;
    float4 tangent  : TANGENT;
    float2 uv       : TEXCOORD;
};
*/
float4 PSMain(PSInput input) : SV_Target
{
    float3 V = normalize(gCameraPos - input.worldPos);

    float3 N0 = normalize(input.normal);
    float3 T = normalize(input.tangent.xyz - N0 * dot(input.tangent.xyz, N0));
    float3 B = cross(N0, T) * input.tangent.w;
    
    
    float2 uv = input.uv;
    if(gHeightScale > 0.0f)
        uv = ParallaxOcclusion(input.uv, float3(dot(V, T), dot(V, B), dot(V, N0)));
    
    float3 N = SampleNormal(uv, T, B, N0);

    float3 albedo = gAlbedoMap.Sample(gSampler, uv).rgb * gAlbedo;
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
