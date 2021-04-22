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

#include "GFSDK_SSAO.h"
#include "shaders/src/SharedDefines.h"

#include <assert.h>
#include <stdexcept>
#include <string.h>

#if _WIN32
#define snprintf sprintf_s
#endif

#if _DEBUG
#define ASSERT(exp)         assert(exp)
#else
#define ASSERT(exp)         ((void)0)
#endif

#ifndef ANDROID
#define ENABLE_EXCEPTIONS 1
#endif

#if ENABLE_EXCEPTIONS
#define THROW_RUNTIME_ERROR()   throw std::runtime_error("")
#else
#define THROW_RUNTIME_ERROR()   ASSERT(0)
#endif

#define THROW_IF(exp)           { if (exp) THROW_RUNTIME_ERROR(); }
#define THROW_IF_FAILED(exp)    { if (exp != S_OK) THROW_RUNTIME_ERROR(); }
#define SAFE_D3D_CALL(exp)      { if (exp != S_OK) ASSERT(0); }

#define SIZEOF_ARRAY(A)         (sizeof(A) / sizeof(A[0]))
#define ZERO_ARRAY(A)           memset(A, 0, sizeof(A));
#define ZERO_STRUCT(S)          memset(&S, 0, sizeof(S));

#define ALIGNED_SIZE(size, align) ((size + (align - 1)) & ~(align - 1))

#define SAFE_DELETE(p)          { if (p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)         { if (p) { (p)->Release(); (p)=NULL; } }
#define SAFE_RELEASE_ARRAY(A)   { for (UINT i = 0; i < SIZEOF_ARRAY(A); ++i) { SAFE_RELEASE(A[i]); } }

#undef STRINGIFY
#define STRINGIFY(A) #A

#undef STRINGIFY_MACRO
#define STRINGIFY_MACRO(s) STRINGIFY(s)

typedef GFSDK_SSAO_UINT UINT;
typedef GFSDK_SSAO_FLOAT FLOAT;

#if SUPPORT_D3D11
#include <d3d11.h>
#endif

#if SUPPORT_D3D12
#include "Common_DX12.h"
#endif

#include "MathUtil.h"
