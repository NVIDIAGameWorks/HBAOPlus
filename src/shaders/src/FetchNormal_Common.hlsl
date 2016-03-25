// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright © 2008-2016 NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.
//

#include "ConstantBuffers.hlsl"

#if API_GL
#define FullResNormalTexture    g_t1
#define FullResNormalTextureMS  g_t1
#endif

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
