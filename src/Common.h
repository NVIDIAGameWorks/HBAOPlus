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
#define ASSERT_GL_ERROR(GL) assert(!GL.glGetError())
#else
#define ASSERT(exp)         ((void)0)
#define ASSERT_GL_ERROR(GL)
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

#if SUPPORT_GL
#include "Common_GL.h"
#endif

#if SUPPORT_D3D12
#include "Common_DX12.h"
#endif

#include "MathUtil.h"
