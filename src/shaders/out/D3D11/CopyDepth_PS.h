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

struct CopyDepth_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    PixelShader& Get(ShaderPermutations::RESOLVE_DEPTH A, ShaderPermutations::DEPTH_LAYER_COUNT B)
    {
        return m_Shader[A][B];
    }

private:
    PixelShader m_Shader[ShaderPermutations::RESOLVE_DEPTH_COUNT][ShaderPermutations::DEPTH_LAYER_COUNT_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::RESOLVE_DEPTH_COUNT == 2, "");
    static_assert(ShaderPermutations::DEPTH_LAYER_COUNT_COUNT == 2, "");
#endif
};

};
