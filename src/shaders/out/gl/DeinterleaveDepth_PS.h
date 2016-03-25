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
    GLSLPrograms::DeinterleaveDepth_PS& Get()
    {
        return m_Shader;
    }

private:
    GLSLPrograms::DeinterleaveDepth_PS m_Shader;
#if _WIN32
#endif
};

};
