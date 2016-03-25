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

};

struct DebugAO_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    PixelShader& Get(ShaderPermutations::ENABLE_BLUR A)
    {
        return m_Shader[A];
    }

private:
    PixelShader m_Shader[ShaderPermutations::ENABLE_BLUR_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::ENABLE_BLUR_COUNT == 2, "");
#endif
};

};
