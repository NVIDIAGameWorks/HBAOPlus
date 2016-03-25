//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

#ifndef ENABLE_FOREGROUND_AO_DEFINED
#define ENABLE_FOREGROUND_AO_DEFINED
    enum ENABLE_FOREGROUND_AO
    {
        ENABLE_FOREGROUND_AO_0,
        ENABLE_FOREGROUND_AO_1,
        ENABLE_FOREGROUND_AO_COUNT,
    };
#endif

#ifndef ENABLE_BACKGROUND_AO_DEFINED
#define ENABLE_BACKGROUND_AO_DEFINED
    enum ENABLE_BACKGROUND_AO
    {
        ENABLE_BACKGROUND_AO_0,
        ENABLE_BACKGROUND_AO_1,
        ENABLE_BACKGROUND_AO_COUNT,
    };
#endif

#ifndef ENABLE_DEPTH_THRESHOLD_DEFINED
#define ENABLE_DEPTH_THRESHOLD_DEFINED
    enum ENABLE_DEPTH_THRESHOLD
    {
        ENABLE_DEPTH_THRESHOLD_0,
        ENABLE_DEPTH_THRESHOLD_1,
        ENABLE_DEPTH_THRESHOLD_COUNT,
    };
#endif

#ifndef FETCH_GBUFFER_NORMAL_DEFINED
#define FETCH_GBUFFER_NORMAL_DEFINED
    enum FETCH_GBUFFER_NORMAL
    {
        FETCH_GBUFFER_NORMAL_0,
        FETCH_GBUFFER_NORMAL_1,
        FETCH_GBUFFER_NORMAL_2,
        FETCH_GBUFFER_NORMAL_COUNT,
    };
#endif

};

struct CoarseAO_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    GLSLPrograms::CoarseAO_PS& Get(ShaderPermutations::ENABLE_FOREGROUND_AO A, ShaderPermutations::ENABLE_BACKGROUND_AO B, ShaderPermutations::ENABLE_DEPTH_THRESHOLD C, ShaderPermutations::FETCH_GBUFFER_NORMAL D)
    {
        return m_Shader[A][B][C][D];
    }

private:
    GLSLPrograms::CoarseAO_PS m_Shader[ShaderPermutations::ENABLE_FOREGROUND_AO_COUNT][ShaderPermutations::ENABLE_BACKGROUND_AO_COUNT][ShaderPermutations::ENABLE_DEPTH_THRESHOLD_COUNT][ShaderPermutations::FETCH_GBUFFER_NORMAL_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::ENABLE_FOREGROUND_AO_COUNT == 2, "");
    static_assert(ShaderPermutations::ENABLE_BACKGROUND_AO_COUNT == 2, "");
    static_assert(ShaderPermutations::ENABLE_DEPTH_THRESHOLD_COUNT == 2, "");
    static_assert(ShaderPermutations::FETCH_GBUFFER_NORMAL_COUNT == 3, "");
#endif
};

};
