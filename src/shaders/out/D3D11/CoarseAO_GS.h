//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

};

struct CoarseAO_GS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    GeometryShader& Get()
    {
        return m_Shader;
    }

private:
    GeometryShader m_Shader;
#if _WIN32
#endif
};

};
