//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

};

struct DeinterleaveDepth_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    PixelShader& Get()
    {
        return m_Shader;
    }

private:
    PixelShader m_Shader;
#if _WIN32
#endif
};

};
