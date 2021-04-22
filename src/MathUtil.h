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

#pragma once

#include <algorithm>
#include <math.h>
#include <float.h>

//--------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------
#undef min
#undef max

#undef Min
#define Min(a,b)            std::min(a,b)

#undef Max
#define Max(a,b)            std::max(a,b)

#undef Clamp
#define Clamp(x,a,b)        (Min(Max(x, a), b))

#undef iDivUp
#define iDivUp(a,b)         ((a + b-1) / b)

#undef D3DX_PI
#define D3DX_PI 3.141592654f

#undef EPSILON
#define EPSILON 1.e-6f

//--------------------------------------------------------------------------------
// Types
//--------------------------------------------------------------------------------
namespace GFSDK
{
namespace SSAO
{

struct float2
{
    float X,Y;
};

struct float3
{
    float X,Y,Z;
};

struct float4
{
    float X,Y,Z,W;
};

struct float4x4
{
    float Data[4*4];
};

struct uint4
{
    GFSDK_SSAO_UINT X,Y,Z,W;
    uint4() {}
    uint4(const GFSDK_SSAO_Version& V)
    {
        X = V.Major;
        Y = V.Minor;
        Z = V.Branch;
        W = V.Revision;
    }
};

} // namespace SSAO
} // namespace GFSDK
