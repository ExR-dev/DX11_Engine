#pragma once

#include "MeshD3D11.h"


[[nodiscard]] bool LoadMeshFromFile(const char *path, MeshData &meshData);
[[nodiscard]] bool WriteMeshToFile(const char *path, const MeshData &meshData);

[[nodiscard]] bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<unsigned char> &data);
