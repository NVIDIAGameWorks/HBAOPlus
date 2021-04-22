/*
#permutation FETCH_GBUFFER_NORMAL 0 1 2
*/

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

#include "ReconstructNormal_Common.hlsl"

//----------------------------------------------------------------------------------
float3 FetchFullResViewNormal(PostProc_VSOut IN, float3 ViewPosition)
{
#if FETCH_GBUFFER_NORMAL
    return FetchFullResViewNormal_GBuffer(IN);
#else
    return ReconstructNormal(IN.uv, ViewPosition);
#endif
}

//----------------------------------------------------------------------------------
float4 DebugNormals_PS(PostProc_VSOut IN) : SV_TARGET
{
    SubtractViewportOrigin(IN);

    float3 ViewPosition = FetchFullResViewPos(IN.uv);
    float3 ViewNormal = -FetchFullResViewNormal(IN, ViewPosition);

    float3 OutColor = 
        (g_iDebugNormalComponent == 0) ? ViewNormal.xxx :
        (g_iDebugNormalComponent == 1) ? ViewNormal.yyy :
        (g_iDebugNormalComponent == 2) ? ViewNormal.zzz :
        ViewNormal;

    return float4(OutColor, 0);
}
