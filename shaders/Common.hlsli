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
};

cbuffer PerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
};
