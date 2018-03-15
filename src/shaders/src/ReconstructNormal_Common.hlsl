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
#include "FetchNormal_Common.hlsl"

Texture2D<float>  FullResDepthTexture       : register(t0);
sampler           PointClampSampler         : register(s0);

//----------------------------------------------------------------------------------
float3 UVToView(float2 UV, float viewDepth)
{
    UV = g_f2UVToViewA * UV + g_f2UVToViewB;
    return float3(UV * viewDepth, viewDepth);
}

//----------------------------------------------------------------------------------
float3 FetchFullResViewPos(float2 UV)
{
    float ViewDepth = FullResDepthTexture.SampleLevel(PointClampSampler, UV, 0);
    return UVToView(UV, ViewDepth);
}

//----------------------------------------------------------------------------------
float3 MinDiff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

//----------------------------------------------------------------------------------
float3 ReconstructNormal(float2 UV, float3 P)
{
    float3 Pr = FetchFullResViewPos(UV + float2(g_f2InvFullResolution.x, 0));
    float3 Pl = FetchFullResViewPos(UV + float2(-g_f2InvFullResolution.x, 0));
    float3 Pt = FetchFullResViewPos(UV + float2(0, g_f2InvFullResolution.y));
    float3 Pb = FetchFullResViewPos(UV + float2(0, -g_f2InvFullResolution.y));
    return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}
