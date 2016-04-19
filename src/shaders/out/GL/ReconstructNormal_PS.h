//! This file was auto-generated. Do not modify manually.
#pragma once

namespace Generated
{

namespace ShaderPermutations
{

};

struct ReconstructNormal_PS
{
    void Create(DevicePointer Device);
    void Release(DevicePointer Device);
    GLSLPrograms::ReconstructNormal_PS& Get()
    {
        return m_Shader;
    }

private:
    GLSLPrograms::ReconstructNormal_PS m_Shader;
#if _WIN32
#endif
};

};
