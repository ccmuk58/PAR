cbuffer PerFrame : register(b0)
{
    float4x4 gViewProj;
    float4x4 gLightViewProj;
    float3   gCameraPos;
    float    gPad0;
    
    float3   gLightDir;
    float    gPad1;
    float3   gLightColor;
    float    gPad2;
    float3   gAmbientColor;
    float    gPad3;

    float3   gPointPos;
    float    gPointRange;
    float3   gPointColor;
    float    gPad4;

    float3   gSpotPos;
    float    gSpotRange;
    float3   gSpotDir;
    float    gSpotCosInner;
    float3   gSpotColor;
    float    gSpotCosOuter;
};

cbuffer PerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
};

cbuffer PerMaterial : register(b2)
{
    float3   gAlbedo;
    float    gShininess;
    float3   gSpecular;
    float    gPad5;
};
