//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

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

struct DeinterleaveDepth_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    PixelShader& Get(ShaderPermutations::DEPTH_LAYER_COUNT A)
    {
        return m_Shader[A];
    }

private:
    PixelShader m_Shader[ShaderPermutations::DEPTH_LAYER_COUNT_COUNT];
#if _WIN32
    static_assert(ShaderPermutations::DEPTH_LAYER_COUNT_COUNT == 2, "");
#endif
};

};
