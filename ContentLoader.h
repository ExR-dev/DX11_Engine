#pragma once

#include "ToDistribute/MeshD3D11.h"

bool LoadMeshFromFile(const char *path, MeshData &meshData);
bool WriteMeshToFile(const char *path, MeshData &meshData);
