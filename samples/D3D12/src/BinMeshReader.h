/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#pragma once

bool LoadVertices(const WCHAR* FileName, std::vector<Vertex>& OutVertices)
{
	FILE* fp = _wfopen(FileName, L"rb");
	if (!fp) return false;

	fseek(fp, 0L, SEEK_END);
	UINT FileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	UINT NumVertices = FileSize / sizeof(Vertex::position);

	for (UINT Idx = 0; Idx < NumVertices; ++Idx)
	{
		Vertex vertex;
		fread(&vertex.position, sizeof(XMFLOAT3), 1, fp);
		vertex.normal.x = vertex.normal.y = 0;
		vertex.normal.z = 1.0;
		vertex.textureCoordinate.x = vertex.textureCoordinate.y = 0;
		OutVertices.push_back(vertex);
	}

	fclose(fp);
	return true;
}

bool LoadIndices(const WCHAR* FileName, std::vector<uint32_t>& OutIndices)
{
	FILE* fp = _wfopen(FileName, L"rb");
	if (!fp) return false;

	fseek(fp, 0L, SEEK_END);
	UINT FileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	UINT NumIndices = FileSize / sizeof(uint32_t);

	for (UINT Idx = 0; Idx < NumIndices; ++Idx)
	{
		uint32_t index;
		fread(&index, sizeof(index), 1, fp);
		OutIndices.push_back(index);
	}

	fclose(fp);
	return true;
}


