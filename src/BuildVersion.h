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

#include "Common.h"

#ifdef ANDROID
// Workaround for a compiler bug on Android
#define BUILD_VERSION_CHECK_MAJOR 0
#define BUILD_VERSION_CHECK_MINOR 0
#define BUILD_VERSION_CHECK_BRANCH 0
#else
#define BUILD_VERSION_CHECK_MAJOR 1
#define BUILD_VERSION_CHECK_MINOR 1
#define BUILD_VERSION_CHECK_BRANCH 1
#endif

namespace GFSDK
{
namespace SSAO
{

class BuildVersion : public GFSDK_SSAO_Version
{
public:
    bool Match(const GFSDK_SSAO_Version &V)
    {
        return 
#if BUILD_VERSION_CHECK_MAJOR
            V.Major     == Major &&
#endif
#if BUILD_VERSION_CHECK_MINOR
            V.Minor     == Minor &&
#endif
#if BUILD_VERSION_CHECK_BRANCH
            V.Branch    == Branch &&
#endif
            V.Revision  == Revision;
    }
};

} // namespace SSAO
} // namespace GFSDK
