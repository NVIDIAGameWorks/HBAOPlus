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

};

struct DebugNormals_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    GLSLPrograms::DebugNormals_PS& Get(ShaderPermutations::FETCH_GBUFFER_NORMAL A)
    {
        return m_Shader[A];
    }

private:
    GLSLPrograms::DebugNormals_PS m_Shader[ShaderPermutations::FETCH_GBUFFER_NORMAL_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::FETCH_GBUFFER_NORMAL_COUNT == 3, "");
#endif
};

};
