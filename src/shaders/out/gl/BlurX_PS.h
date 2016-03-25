//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

#ifndef ENABLE_SHARPNESS_PROFILE_DEFINED
#define ENABLE_SHARPNESS_PROFILE_DEFINED
    enum ENABLE_SHARPNESS_PROFILE
    {
        ENABLE_SHARPNESS_PROFILE_0,
        ENABLE_SHARPNESS_PROFILE_1,
        ENABLE_SHARPNESS_PROFILE_COUNT,
    };
#endif

#ifndef KERNEL_RADIUS_DEFINED
#define KERNEL_RADIUS_DEFINED
    enum KERNEL_RADIUS
    {
        KERNEL_RADIUS_2,
        KERNEL_RADIUS_4,
        KERNEL_RADIUS_COUNT,
    };
#endif

};

struct BlurX_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    GLSLPrograms::BlurX_PS& Get(ShaderPermutations::ENABLE_SHARPNESS_PROFILE A, ShaderPermutations::KERNEL_RADIUS B)
    {
        return m_Shader[A][B];
    }

private:
    GLSLPrograms::BlurX_PS m_Shader[ShaderPermutations::ENABLE_SHARPNESS_PROFILE_COUNT][ShaderPermutations::KERNEL_RADIUS_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::ENABLE_SHARPNESS_PROFILE_COUNT == 2, "");
    static_assert(ShaderPermutations::KERNEL_RADIUS_COUNT == 2, "");
#endif
};

};
