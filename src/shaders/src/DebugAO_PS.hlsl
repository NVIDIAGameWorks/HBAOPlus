/*
#permutation ENABLE_BLUR 0 1
*/

/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "ConstantBuffers.hlsl"
#include "FullScreenTriangle_VS.hlsl"

#define g_AOResolution g_FullResolution
#define g_InvAOResolution g_f2InvFullResolution

#define QUARTER_RES_STEPS 1

Texture2D<float>  LinearDepthTexture    : register(t0);
Texture2D<float4> RandomTexture         : register(t1);
sampler PointClampSampler               : register(s0);
sampler PointWrapSampler                : register(s1);

//----------------------------------------------------------------------------------
float InvLength(float2 v)
{
    return rsqrt(dot(v,v));
}

//----------------------------------------------------------------------------------
float3 UVToEye(float2 UV, float ViewDepth)
{
    UV = g_f2UVToViewA * UV + g_f2UVToViewB;
    return float3(UV * ViewDepth, ViewDepth);
}

//----------------------------------------------------------------------------------
float3 FetchEyePos(float2 UV)
{
    float HardwareDepth = LinearDepthTexture.SampleLevel(PointClampSampler, UV, 0);
    return UVToEye(UV, HardwareDepth);
}

//----------------------------------------------------------------------------------
float3 MinDiff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

//----------------------------------------------------------------------------------
float Falloff(float DistanceSquare)
{
    // 1 scalar mad instruction
    return DistanceSquare * g_fNegInvR2 + 1.0f;
}

//----------------------------------------------------------------------------------
float2 RotateDirection(float2 Dir, float2 CosSin)
{
    return float2(Dir.x*CosSin.x - Dir.y*CosSin.y,
                  Dir.x*CosSin.y + Dir.y*CosSin.x);
}

//----------------------------------------------------------------------------------
float ComputeAO(float3 P, float3 N, float3 S)
{
    float3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) * rsqrt(VdotV);
    return saturate(NdotV - g_fNDotVBias) * saturate(Falloff(VdotV));
}

//----------------------------------------------------------------------------------
#if ENABLE_BLUR
float2 DebugAO_PS(PostProc_VSOut IN) : SV_TARGET
#else
float4 DebugAO_PS(PostProc_VSOut IN) : SV_TARGET
#endif
{
    float3 P = FetchEyePos(IN.uv);
    float BlurDepth = P.z;

#if USE_RANDOM_TEXTURE
    // (cos(Alpha),sin(Alpha),rand1,rand2)
    float4 Rand = RandomTexture.Sample(PointWrapSampler, IN.pos.xy / RANDOM_TEXTURE_WIDTH);
#else
    float4 Rand = float4(1,0,1,1);
#endif

    // Compute projection of disk of radius g_R into screen space
    float RadiusPixels = g_fRadiusToScreen / P.z;

    // Early exit if the radius is smaller than one pixel
    if (RadiusPixels < 1.0)
    {
#if ENABLE_BLUR
        return float2(1.0, BlurDepth);
#else
        return 1.0;
#endif
    }

#if QUARTER_RES_STEPS
    RadiusPixels /= 4;
#endif

    // Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
    float StepSizePixels = RadiusPixels / (NUM_STEPS + 1);

    // Reconstruct view-space normal from nearest neighbors
    float3 Pr = FetchEyePos(IN.uv + float2(g_InvAOResolution.x, 0));
    float3 Pl = FetchEyePos(IN.uv + float2(-g_InvAOResolution.x, 0));
    float3 Pt = FetchEyePos(IN.uv + float2(0, g_InvAOResolution.y));
    float3 Pb = FetchEyePos(IN.uv + float2(0, -g_InvAOResolution.y));
    float3 N = normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));

    const float Alpha = 2.0 * GFSDK_PI / NUM_DIRECTIONS;
    float SmallScaleAO = 0;
    float LargeScaleAO = 0;

    [unroll]
    for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
    {
        float Angle = Alpha * DirectionIndex;

        // Compute normalized 2D direction
        float2 dir = RotateDirection(float2(cos(Angle), sin(Angle)), Rand.xy);

        // Jitter starting point within the first step
        float RayPixels = (Rand.z * StepSizePixels + 1.0);

        [unroll]
        for (float StepIndex = 0; StepIndex < NUM_STEPS; ++StepIndex)
        {
#if QUARTER_RES_STEPS
            float2 snappedUV = round(RayPixels * dir) * (g_InvAOResolution * 4.0) + IN.uv;
#else
            float2 snappedUV = round(RayPixels * dir) * g_InvAOResolution + IN.uv;
#endif
            float3 S = FetchEyePos(snappedUV);
            RayPixels += StepSizePixels;

            if (StepIndex == 0)
            {
                SmallScaleAO += ComputeAO(P, N, S);
            }
            else
            {
                LargeScaleAO += ComputeAO(P, N, S);
            }
        }
    }

    float AO = (SmallScaleAO * g_fSmallScaleAOAmount) + (LargeScaleAO * g_fLargeScaleAOAmount);
    AO /= (NUM_DIRECTIONS * NUM_STEPS);
    AO = saturate(1.0 - AO * 2.0);

#if ENABLE_BLUR
    return float2(AO, BlurDepth);
#else
    return pow(saturate(AO), g_fPowExponent);
#endif
}
