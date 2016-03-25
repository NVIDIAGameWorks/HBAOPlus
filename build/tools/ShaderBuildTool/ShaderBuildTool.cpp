// ShaderBuildTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <sstream>

#define PERMUTATION_KEYWORD "#permutation "
#define SEPARATORS " \n"
#define INDENT "    "

namespace
{
    struct CShaderPermutation
    {
        std::string Key;
        std::vector<std::string> Values;
    };

    struct CShaderDefine
    {
        std::string Key;
        std::string Value;
    };

    struct CShaderInstance
    {
        std::string GlobalVariableName;
        std::vector<CShaderDefine> Defines;
    };

    typedef std::vector<CShaderInstance> CShaderInstances;
    typedef std::vector<CShaderPermutation> CShaderPermutations;
};

FILE *OpenFile(std::string Path, const char* Mode)
{
    FILE *fp = NULL;
    if (fopen_s(&fp, Path.c_str(), Mode) || !fp)
    {
        fprintf(stderr, "Failed to open file %s\n", Path.c_str());
        exit(1);
    }
    return fp;
}

CShaderInstances ExpandShaderInstances(CShaderInstances In, CShaderPermutation Permutation)
{
    CShaderInstances Out;
    for (unsigned int InstanceIndex = 0; InstanceIndex < In.size(); ++InstanceIndex)
    {
        for (unsigned int ValueIndex = 0; ValueIndex < Permutation.Values.size(); ++ValueIndex)
        {
            CShaderDefine D;
            D.Key = Permutation.Key;
            D.Value = Permutation.Values[ValueIndex];

            CShaderInstance Tmp = In[InstanceIndex];
            Tmp.Defines.push_back(D);
            Out.push_back(Tmp);
        }
    }

    return Out;
}

void MySystem(const char *Command)
{
    //printf("%s\n", Command);

    if (system(Command) != 0)
    {
        fprintf(stderr, "Failed command: %s\n", Command);
        exit(1);
    }
}

struct CArgs
{
    std::string Profile;
    std::string Input;
    std::string EntryPoint;
    std::string OutputDir;
    std::string Output_H;
    std::string Output_CPP;
    std::string CompileTool;
    std::string API;
    std::string ClassName;
};

void ParseInput(
    const CArgs& Args,
    CShaderPermutations& ShaderPermutations)
{
    FILE *fp = OpenFile(Args.Input, "r");

    char row[1024];
    while (fgets(row, sizeof(row), fp))
    {
        if (strncmp(row, PERMUTATION_KEYWORD, strlen(PERMUTATION_KEYWORD)) == 0)
        {
            char *Keyword = strtok(row, SEPARATORS);
            char *DefineKey = strtok(NULL, SEPARATORS);
            if (!DefineKey) break;

            CShaderPermutation P;
            P.Key = DefineKey;

            while (1)
            {
                char *DefineValue = strtok(NULL, SEPARATORS);
                if (!DefineValue) break;

                P.Values.push_back(DefineValue);
            }

            ShaderPermutations.push_back(P);
        }
    }

    fclose(fp);
}

void InitShaderInstances(
    const CShaderPermutations& ShaderPermutations,
    CShaderInstances& ShaderInstances
    )
{
    ShaderInstances.clear();
    ShaderInstances.push_back(CShaderInstance());

    for (unsigned int PermutationIndex = 0; PermutationIndex < ShaderPermutations.size(); ++PermutationIndex)
    {
        CShaderPermutation P = ShaderPermutations[PermutationIndex];
        ShaderInstances = ExpandShaderInstances(ShaderInstances, P);
    }
}

void WriteCPP(
    const CArgs& Args,
    const CShaderPermutations& ShaderPermutations,
    CShaderInstances& ShaderInstances)
{
    remove(Args.Output_CPP.c_str());

    for (unsigned int InstanceIndex = 0; InstanceIndex < ShaderInstances.size(); ++InstanceIndex)
    {
        CShaderInstance& Instance = ShaderInstances[InstanceIndex];
        std::string DefineString = "/DAPI_" + Args.API + "=1";
        std::string ShaderName = Args.EntryPoint;

        for (unsigned int i = 0; i < ShaderInstances[i].Defines.size(); ++i)
        {
            DefineString += " /D" + Instance.Defines[i].Key + "=" + Instance.Defines[i].Value;
            ShaderName += "_" + Instance.Defines[i].Key + "_" + Instance.Defines[i].Value;
        }

        Instance.GlobalVariableName = "g_" + ShaderName + "_" + Args.API;

        std::string OutputTmp = Args.Output_CPP + ".tmp";
        std::string Command;
        Command += "SET SBT_PROFILE=" + Args.Profile;
        Command += "& SET SBT_INPUT=" + Args.Input;
        Command += "& SET SBT_ENTRY_POINT=" + Args.EntryPoint;
        Command += "& SET SBT_DEFINES=" + DefineString;
        Command += "& SET SBT_VARIABLE_NAME=" + Instance.GlobalVariableName;
        Command += "& SET SBT_OUTPUT=" + OutputTmp;
        Command += "& " + Args.CompileTool;
        MySystem(Command.c_str());

        Command = "type " + OutputTmp + " >> " + Args.Output_CPP;
        MySystem(Command.c_str());

        Command = "del " + OutputTmp;
        MySystem(Command.c_str());
    }

    FILE *fp = OpenFile(Args.Output_CPP, "a+");

    fprintf(fp, "\n");
    fprintf(fp, "namespace Generated\n");
    fprintf(fp, "{\n");

    fprintf(fp, INDENT "void %s::Create(DevicePointer Device)\n", Args.EntryPoint.c_str());
    fprintf(fp, INDENT "{\n");
    for (unsigned int i = 0; i < ShaderInstances.size(); ++i)
    {
        CShaderInstance Instance = ShaderInstances[i];

        fprintf(fp, INDENT INDENT "m_Shader");
        for (unsigned int j = 0; j < Instance.Defines.size(); ++j)
        {
            fprintf(fp, "[ShaderPermutations::%s", Instance.Defines[j].Key.c_str());
            fprintf(fp, "_%s]", Instance.Defines[j].Value.c_str());
        }
        fprintf(fp, ".Create(Device, %s, sizeof(%s));\n", Instance.GlobalVariableName.c_str(), Instance.GlobalVariableName.c_str());
    }
    fprintf(fp, INDENT "}\n");
    fprintf(fp, "\n");

    fprintf(fp, INDENT "void %s::Release(DevicePointer Device)\n", Args.EntryPoint.c_str());
    fprintf(fp, INDENT "{\n");
    for (unsigned int i = 0; i < ShaderInstances.size(); ++i)
    {
        fprintf(fp, INDENT INDENT "m_Shader");
        CShaderInstance Instance = ShaderInstances[i];
        for (unsigned int j = 0; j < Instance.Defines.size(); ++j)
        {
            fprintf(fp, "[ShaderPermutations::%s", Instance.Defines[j].Key.c_str());
            fprintf(fp, "_%s]", Instance.Defines[j].Value.c_str());
        }
        fprintf(fp, ".Release(Device);\n");
    }
    fprintf(fp, INDENT "}\n");

    fprintf(fp, "}\n");

    fclose(fp);
}

void WriteHeader(
    const CArgs& Args,
    const CShaderPermutations& ShaderPermutations,
    const CShaderInstances& ShaderInstances)
{
    remove(Args.Output_H.c_str());

    std::string ClassName = Args.ClassName;

    FILE *fp = OpenFile(Args.Output_H, "a+");

    fprintf(fp, "//! This file was auto-generated. Do not modify manually.\n");
    fprintf(fp, "#pragma once");
    fprintf(fp, "\n");

    fprintf(fp, "\n");
    fprintf(fp, "namespace Generated\n");
    fprintf(fp, "{\n");
    fprintf(fp, "\n");

    fprintf(fp, "namespace ShaderPermutations\n");
    fprintf(fp, "{\n");
    fprintf(fp, "\n");

    for (unsigned int i = 0; i < ShaderPermutations.size(); ++i)
    {
        const CShaderPermutation& Permutation = ShaderPermutations[i];

        std::string PermutationDefine = Permutation.Key + "_DEFINED";
        fprintf(fp, "#ifndef %s\n", PermutationDefine.c_str());
        fprintf(fp, "#define %s\n", PermutationDefine.c_str());

        fprintf(fp, INDENT "enum %s\n", Permutation.Key.c_str());
        fprintf(fp, INDENT "{\n");
        for (unsigned int j = 0; j < Permutation.Values.size(); ++j)
        {
            fprintf(fp, INDENT INDENT "%s_%s,\n", Permutation.Key.c_str(), Permutation.Values[j].c_str());
        }
        fprintf(fp, INDENT INDENT "%s_COUNT,\n", Permutation.Key.c_str());
        fprintf(fp, INDENT "};\n");

        fprintf(fp, "#endif\n");
        fprintf(fp, "\n");
    }

    fprintf(fp, "};\n");
    fprintf(fp, "\n");

    fprintf(fp, "struct %s\n", Args.EntryPoint.c_str());
    fprintf(fp, "{\n");

    fprintf(fp, INDENT "void Create(DevicePointer Device);\n");
    fprintf(fp, INDENT "void Release(DevicePointer Device);\n");

    fprintf(fp, INDENT "%s& Get(", ClassName.c_str());
    for (unsigned int i = 0; i < ShaderPermutations.size(); ++i)
    {
        if (i != 0)
        {
            fprintf(fp, ", ");
        }
        fprintf(fp, "ShaderPermutations::%s %c", ShaderPermutations[i].Key.c_str(), 'A' + i);
    }
    fprintf(fp, ")\n");
    fprintf(fp, INDENT "{\n");
    fprintf(fp, INDENT INDENT "return m_Shader");
    for (unsigned int i = 0; i < ShaderPermutations.size(); ++i)
    {
        fprintf(fp, "[%c]", 'A' + i);
    }
    fprintf(fp, ";\n");
    fprintf(fp, INDENT "}\n");

    fprintf(fp, "\n");
    fprintf(fp, "private:\n");

    fprintf(fp, INDENT "%s m_Shader", ClassName.c_str());
    for (unsigned int i = 0; i < ShaderPermutations.size(); ++i)
    {
        fprintf(fp, "[ShaderPermutations::%s_COUNT]", ShaderPermutations[i].Key.c_str());
    }
    fprintf(fp, ";\n");

    fprintf(fp, "#if _WIN32\n");
    for (unsigned int i = 0; i < ShaderPermutations.size(); ++i)
    {
        fprintf(fp, INDENT "static_assert(ShaderPermutations::%s_COUNT == %d, \"\");\n",
            ShaderPermutations[i].Key.c_str(),
            (int) ShaderPermutations[i].Values.size());
    }
    fprintf(fp, "#endif\n");

    fprintf(fp, "};\n");
    fprintf(fp, "\n");

    fprintf(fp, "};\n");

    fclose(fp);
}

int main(int argc, char **argv)
{
    if (argc < 7)
    {
        fprintf(stderr, "Usage: %s PROFILE INPUT_HLSL ENTRY_POINT OUTPUT_DIR COMPILE_TOOL API [CLASS_NAME]\n", argv[0]);
        exit(1);
    }

    CArgs Args;
    int ArgIndex = 1;
    if (ArgIndex < argc)
    {
        Args.Profile = argv[ArgIndex++];
    }
    if (ArgIndex < argc)
    {
        Args.Input = argv[ArgIndex++];
    }
    if (ArgIndex < argc)
    {
        Args.EntryPoint = argv[ArgIndex++];
    }
    if (ArgIndex < argc)
    {
        Args.OutputDir = argv[ArgIndex++];
    }
    if (ArgIndex < argc)
    {
        Args.CompileTool = argv[ArgIndex++];
    }
    if (ArgIndex < argc)
    {
        Args.API = argv[ArgIndex++];
    }
    if (ArgIndex < argc)
    {
        Args.ClassName = argv[ArgIndex++];
    }

    if (Args.ClassName.size() == 0)
    {
        const bool IsVS = !strcmp(Args.Profile.c_str(), "vs_5_0");
        const bool IsGS = !strcmp(Args.Profile.c_str(), "gs_5_0");
        Args.ClassName =
            IsVS ? "VertexShader" :
            IsGS ? "GeometryShader" :
            "PixelShader";
    }

    CShaderPermutations ShaderPermutations;
    ParseInput(Args, ShaderPermutations);

    Args.Output_H = Args.OutputDir + "\\" + Args.EntryPoint + ".h";
    Args.Output_CPP = Args.OutputDir + "\\" + Args.EntryPoint + ".cpp";

    CShaderInstances ShaderInstances;
    InitShaderInstances(ShaderPermutations, ShaderInstances);

    WriteHeader(Args, ShaderPermutations, ShaderInstances);

    WriteCPP(Args, ShaderPermutations, ShaderInstances);

    return 0;
}
