/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdint.h>

class Mesh
{
public:
    struct Vertex
    {
        float Position[3];
    };

    static std::shared_ptr<Mesh> LoadFromFile(const std::string& filename);

//protected:
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;
};