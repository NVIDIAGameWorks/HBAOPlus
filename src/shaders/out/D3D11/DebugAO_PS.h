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

struct DebugAO_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    PixelShader& Get(ShaderPermutations::ENABLE_BLUR A, ShaderPermutations::NUM_STEPS B)
    {
        return m_Shader[A][B];
    }

private:
    PixelShader m_Shader[ShaderPermutations::ENABLE_BLUR_COUNT][ShaderPermutations::NUM_STEPS_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::ENABLE_BLUR_COUNT == 2, "");
    static_assert(ShaderPermutations::NUM_STEPS_COUNT == 2, "");
#endif
};

};
