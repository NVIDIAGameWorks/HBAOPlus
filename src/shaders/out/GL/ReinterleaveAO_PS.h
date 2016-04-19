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

struct ReinterleaveAO_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    GLSLPrograms::ReinterleaveAO_PS& Get(ShaderPermutations::ENABLE_BLUR A)
    {
        return m_Shader[A];
    }

private:
    GLSLPrograms::ReinterleaveAO_PS m_Shader[ShaderPermutations::ENABLE_BLUR_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::ENABLE_BLUR_COUNT == 2, "");
#endif
};

};
