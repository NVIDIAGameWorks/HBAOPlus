//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

};

struct FullScreenTriangle_VS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    VertexShader& Get()
    {
        return m_Shader;
    }

private:
    VertexShader m_Shader;
#if _WIN32
#endif
};

};
