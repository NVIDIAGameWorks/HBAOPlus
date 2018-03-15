//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

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

#ifndef DEPTH_LAYER_COUNT_DEFINED
#define DEPTH_LAYER_COUNT_DEFINED
    enum DEPTH_LAYER_COUNT
    {
        DEPTH_LAYER_COUNT_1,
        DEPTH_LAYER_COUNT_2,
        DEPTH_LAYER_COUNT_COUNT,
    };
#endif

#ifndef NUM_STEPS_DEFINED
#define NUM_STEPS_DEFINED
    enum NUM_STEPS
    {
        NUM_STEPS_4,
        NUM_STEPS_8,
        NUM_STEPS_COUNT,
    };
#endif

};

struct CoarseAO_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    PixelShader& Get(ShaderPermutations::FETCH_GBUFFER_NORMAL A, ShaderPermutations::DEPTH_LAYER_COUNT B, ShaderPermutations::NUM_STEPS C)
    {
        return m_Shader[A][B][C];
    }

private:
    PixelShader m_Shader[ShaderPermutations::FETCH_GBUFFER_NORMAL_COUNT][ShaderPermutations::DEPTH_LAYER_COUNT_COUNT][ShaderPermutations::NUM_STEPS_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::FETCH_GBUFFER_NORMAL_COUNT == 3, "");
    static_assert(ShaderPermutations::DEPTH_LAYER_COUNT_COUNT == 2, "");
    static_assert(ShaderPermutations::NUM_STEPS_COUNT == 2, "");
#endif
};

};
