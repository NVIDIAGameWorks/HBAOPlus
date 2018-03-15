/*
#permutation FETCH_GBUFFER_NORMAL 0 1 2
#permutation DEPTH_LAYER_COUNT 1 2
#permutation NUM_STEPS 4 8
*/

/* 
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "FullScreenTriangle_VS.hlsl"

#if FETCH_GBUFFER_NORMAL
#include "FetchNormal_Common.hlsl"
#endif

#if DEPTH_LAYER_COUNT==2
Texture2DArray<float2>   QuarterResDepthTexture      : register(t0);
#else
Texture2DArray<float>   QuarterResDepthTexture      : register(t0);
#endif

#if !FETCH_GBUFFER_NORMAL
Texture2D<float3>       ReconstructedNormalTexture  : register(t1);
#endif

sampler                 PointClampSampler           : register(s0);

//----------------------------------------------------------------------------------
float3 UVToView(float2 UV, float ViewDepth)
{
    UV = g_f2UVToViewA * UV + g_f2UVToViewB;
    return float3(UV * ViewDepth, ViewDepth);
}

//----------------------------------------------------------------------------------
float3 FetchFullResViewNormal(PostProc_VSOut IN)
{
#if !FETCH_GBUFFER_NORMAL
    return ReconstructedNormalTexture.Load(int3(IN.pos.xy,0)) * 2.0 - 1.0;
#else
    return FetchFullResViewNormal_GBuffer(IN);
#endif
}

//----------------------------------------------------------------------------------
#if DEPTH_LAYER_COUNT==2
void FetchQuarterResViewPos(float2 UV, out float3 OutViewPos0, out float3 OutViewPos1)
{
    float2 ViewDepths = QuarterResDepthTexture.SampleLevel(PointClampSampler, float3(UV, 0), 0).rg;
    OutViewPos0 = UVToView(UV, ViewDepths.r);
    OutViewPos1 = UVToView(UV, ViewDepths.g);
}
#else
float3 FetchQuarterResViewPos(float2 UV)
{
    float ViewDepth = QuarterResDepthTexture.SampleLevel(PointClampSampler, float3(UV, 0), 0);
    return UVToView(UV, ViewDepth);
}
#endif

//----------------------------------------------------------------------------------
float2 RotateDirection(float2 V, float2 RotationCosSin)
{
    // RotationCosSin is (cos(alpha),sin(alpha)) where alpha is the rotation angle
    // A 2D rotation matrix is applied (see https://en.wikipedia.org/wiki/Rotation_matrix)
    return float2(V.x*RotationCosSin.x - V.y*RotationCosSin.y,
                  V.x*RotationCosSin.y + V.y*RotationCosSin.x);
}

//----------------------------------------------------------------------------------
float DepthThresholdFactor(float ViewDepth)
{
    return saturate((ViewDepth * g_fViewDepthThresholdNegInv + 1.0) * g_fViewDepthThresholdSharpness);
}

//----------------------------------------------------------------------------------
struct AORadiusParams
{
    float fRadiusPixels;
    float fNegInvR2;
};

//----------------------------------------------------------------------------------
void ScaleAORadius(inout AORadiusParams Params, float ScaleFactor)
{
    Params.fRadiusPixels *= ScaleFactor;
    Params.fNegInvR2 *= 1.0 / (ScaleFactor * ScaleFactor);
}

//----------------------------------------------------------------------------------
AORadiusParams GetAORadiusParams(float ViewDepth)
{
    AORadiusParams Params;
    Params.fRadiusPixels = g_fRadiusToScreen / ViewDepth;
    Params.fNegInvR2 = g_fNegInvR2;

    [branch] if (g_fBackgroundAORadiusPixels != -1.f)
    {
        ScaleAORadius(Params, max(1.0, g_fBackgroundAORadiusPixels / Params.fRadiusPixels));
    }

    [branch] if (g_fForegroundAORadiusPixels != -1.f)
    {
        ScaleAORadius(Params, min(1.0, g_fForegroundAORadiusPixels / Params.fRadiusPixels));
    }

    return Params;
}

//----------------------------------------------------------------------------------
float Falloff(float DistanceSquare, AORadiusParams Params)
{
    // 1 scalar mad instruction
    return DistanceSquare * Params.fNegInvR2 + 1.0;
}

//----------------------------------------------------------------------------------
// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
//----------------------------------------------------------------------------------
float ComputeAO(float3 P, float3 N, float3 S, AORadiusParams Params)
{
    float3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) * rsqrt(VdotV);

    // Use saturate(x) instead of max(x,0.f) because that is faster
    return saturate(NdotV - g_fNDotVBias) * saturate(Falloff(VdotV, Params));
}

//----------------------------------------------------------------------------------
void AccumulateAO(
    inout float AO,
    inout float RayPixels,
    float StepSizePixels,
    float2 Direction,
    float2 FullResUV,
    float3 ViewPosition,
    float3 ViewNormal,
    AORadiusParams Params
)
{
    float2 SnappedUV = round(RayPixels * Direction) * g_f2InvQuarterResolution + FullResUV;

#if DEPTH_LAYER_COUNT == 2
    float3 S0, S1;
    FetchQuarterResViewPos(SnappedUV, S0, S1);
#else
    float3 S = FetchQuarterResViewPos(SnappedUV);
#endif

    RayPixels += StepSizePixels;

#if DEPTH_LAYER_COUNT == 2
    AO += max(
        ComputeAO(ViewPosition, ViewNormal, S0, Params),
        ComputeAO(ViewPosition, ViewNormal, S1, Params));
#else
    AO += ComputeAO(ViewPosition, ViewNormal, S, Params);
#endif
}

//----------------------------------------------------------------------------------
float ComputeCoarseAO(float2 FullResUV, float3 ViewPosition, float3 ViewNormal, AORadiusParams Params)
{
    // Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
    float StepSizePixels = (Params.fRadiusPixels / 4.0) / (NUM_STEPS + 1);

#if USE_RANDOM_TEXTURE
    float4 Rand = g_PerPassConstants.f4Jitter;
#else
    float4 Rand = float4(1,0,1,1);
#endif

    const float Alpha = 2.0 * GFSDK_PI / NUM_DIRECTIONS;
    float SmallScaleAO = 0;
    float LargeScaleAO = 0;

    [unroll]
    for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
    {
        float Angle = Alpha * DirectionIndex;

        // Compute normalized 2D direction
        float2 Direction = RotateDirection(float2(cos(Angle), sin(Angle)), Rand.xy);

        // Jitter starting sample within the first step
        float RayPixels = (Rand.z * StepSizePixels + 1.0);

        {
            AccumulateAO(SmallScaleAO, RayPixels, StepSizePixels, Direction, FullResUV, ViewPosition, ViewNormal, Params);
        }

        [unroll]
        for (float StepIndex = 1; StepIndex < NUM_STEPS; ++StepIndex)
        {
            AccumulateAO(LargeScaleAO, RayPixels, StepSizePixels, Direction, FullResUV, ViewPosition, ViewNormal, Params);
        }
    }

    float AO = (SmallScaleAO * g_fSmallScaleAOAmount) + (LargeScaleAO * g_fLargeScaleAOAmount);

    AO /= (NUM_DIRECTIONS * NUM_STEPS);

    return AO;
}

//----------------------------------------------------------------------------------
float CoarseAO_PS(PostProc_VSOut IN) : SV_TARGET
{
    IN.pos.xy = floor(IN.pos.xy) * 4.0 + g_PerPassConstants.f2Offset;
    IN.uv = IN.pos.xy * (g_f2InvQuarterResolution / 4.0);

    // Batch 2 texture fetches before the branch
#if DEPTH_LAYER_COUNT==2
    float3 ViewPosition;
    float3 ViewPosition1;
    FetchQuarterResViewPos(IN.uv, ViewPosition, ViewPosition1);
#else
    float3 ViewPosition = FetchQuarterResViewPos(IN.uv);
#endif // DEPTH_LAYER_COUNT==2

    float3 ViewNormal = FetchFullResViewNormal(IN);

    AORadiusParams Params = GetAORadiusParams(ViewPosition.z);

    // Early exit if the projected radius is smaller than 1 full-res pixel
    [branch]
    if (Params.fRadiusPixels < 1.0)
    {
        return 1.0;
    }

    float AO = ComputeCoarseAO(IN.uv, ViewPosition, ViewNormal, Params);

    [branch]
    if (g_fViewDepthThresholdSharpness != -1.f)
    {
        AO *= DepthThresholdFactor(ViewPosition.z);
    }

    return saturate(1.0 - AO * 2.0);
}
