#pragma once

#include "MeshD3D11.h"


bool LoadMeshFromFile(const char *path, MeshData &meshData);
bool WriteMeshToFile(const char *path, const MeshData &meshData);

bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<unsigned char> &data);
