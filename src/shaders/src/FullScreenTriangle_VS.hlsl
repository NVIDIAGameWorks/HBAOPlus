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

struct PostProc_VSOut
{
    float4 pos  : SV_Position;
    float2 uv   : TEXCOORD0;
};

//----------------------------------------------------------------------------------
// Vertex shader that generates a full-screen triangle with texcoords
// Assuming a Draw(3,0) call.
//----------------------------------------------------------------------------------
PostProc_VSOut FullScreenTriangle_VS( uint VertexId : SV_VertexID )
{
    PostProc_VSOut output = (PostProc_VSOut)0.0f;
    output.uv = float2( (VertexId << 1) & 2, VertexId & 2 );
    output.pos = float4( output.uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f) , 0.0f, 1.0f );
    return output;
}

//----------------------------------------------------------------------------------
void AddViewportOrigin(inout PostProc_VSOut IN)
{
    IN.pos.xy += g_f2InputViewportTopLeft;
    IN.uv = IN.pos.xy * g_f2InvFullResolution;
}

//----------------------------------------------------------------------------------
void SubtractViewportOrigin(inout PostProc_VSOut IN)
{
    IN.pos.xy -= g_f2InputViewportTopLeft;
    IN.uv = IN.pos.xy * g_f2InvFullResolution;
}
