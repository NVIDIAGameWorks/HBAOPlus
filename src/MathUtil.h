/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

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
