//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

#ifndef ENABLE_BLUR_DEFINED
#define ENABLE_BLUR_DEFINED
    enum ENABLE_BLUR
    {
        ENABLE_BLUR_0,
        ENABLE_BLUR_1,
        ENABLE_BLUR_COUNT,
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

};

struct ReinterleaveAO_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    PixelShader& Get(ShaderPermutations::ENABLE_BLUR A, ShaderPermutations::DEPTH_LAYER_COUNT B)
    {
        return m_Shader[A][B];
    }

private:
    PixelShader m_Shader[ShaderPermutations::ENABLE_BLUR_COUNT][ShaderPermutations::DEPTH_LAYER_COUNT_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::ENABLE_BLUR_COUNT == 2, "");
    static_assert(ShaderPermutations::DEPTH_LAYER_COUNT_COUNT == 2, "");
#endif
};

};
