// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved.

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
