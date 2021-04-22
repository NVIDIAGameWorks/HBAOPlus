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

#if FETCH_GBUFFER_NORMAL == 2
Texture2DMS<float3> FullResNormalTextureMS  : register(t1);
#else
Texture2D<float3> FullResNormalTexture      : register(t1);
#endif

//----------------------------------------------------------------------------------
float3 FetchFullResWorldNormal_GBuffer(PostProc_VSOut IN)
{
    AddViewportOrigin(IN);

#if FETCH_GBUFFER_NORMAL == 2
    return FullResNormalTextureMS.Load(int2(IN.pos.xy), g_iSampleIndex).xyz;
#else
    return FullResNormalTexture.Load(int3(IN.pos.xy, 0)).xyz;
#endif
}

//----------------------------------------------------------------------------------
float3 FetchFullResViewNormal_GBuffer(PostProc_VSOut IN)
{
    float3 WorldNormal = FetchFullResWorldNormal_GBuffer(IN) * g_fNormalDecodeScale + g_fNormalDecodeBias;
    float3 ViewNormal = normalize(mul(WorldNormal, (float3x3)g_f44NormalMatrix));
    return ViewNormal;
}
