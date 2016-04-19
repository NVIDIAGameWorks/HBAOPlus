//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

#ifndef RESOLVE_DEPTH_DEFINED
#define RESOLVE_DEPTH_DEFINED
    enum RESOLVE_DEPTH
    {
        RESOLVE_DEPTH_0,
        RESOLVE_DEPTH_1,
        RESOLVE_DEPTH_COUNT,
    };
#endif

};

struct CopyDepth_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    GLSLPrograms::CopyDepth_PS& Get(ShaderPermutations::RESOLVE_DEPTH A)
    {
        return m_Shader[A];
    }

private:
    GLSLPrograms::CopyDepth_PS m_Shader[ShaderPermutations::RESOLVE_DEPTH_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::RESOLVE_DEPTH_COUNT == 2, "");
#endif
};

};
