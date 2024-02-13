#include "ContentLoader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>


struct RawVertex	{ float	x, y, z; };
struct RawTexCoord	{ float	u, v;	 };
struct RawNormal	{ float	x, y, z; };
struct RawIndex		{ int	v, t, n; };


bool LoadMeshFromFile(const char *path, MeshData &meshData)
{
	std::ifstream fileStream(path);
	std::string line;

	std::vector<RawVertex> vertexPositions;
	std::vector<RawTexCoord> vertexTexCoords;
	std::vector<RawNormal> vertexNormals;
	std::vector<RawIndex> indexGroups;

	bool begunReadingSubMesh = false;
	int endOfLastSubMesh = 0;

	while (std::getline(fileStream, line))
	{
		size_t commentStart = line.find_first_of('#');
		if (commentStart != std::string::npos)
			line = line.substr(0, commentStart); // Exclude comments

		if (line.empty())
			continue; // Skip filler line
		
		if (line.length() > 1)
			if (line.at(0) == 'f' && line.at(1) == ' ')
			{
				char sep = '/';
				if (line.find('\\', 0) != std::string::npos)
					sep = '\\';
			
				// Replace all instances of separator with whitespace.
				std::string::size_type n = 0;
				while ((n = line.find(sep, n)) != std::string::npos)
				{ 
					line.replace(n, 1, " ");
					n++;
				}
			}

		std::istringstream segments(line);

		std::string dataType;
		if (!(segments >> dataType))
		{
			std::cerr << "Failed to get data type from line \"" << line << "\"!"<< std::endl;
			return false;
		}

		if (dataType == "mtllib") 
		{ // Define where to find material definitions

		}
		else if (dataType == "g")
		{ // Mesh group
			
		}
		else if (dataType == "o")
		{ // Mesh object
			
		}
		else if (dataType == "v")
		{ // Vertex Point
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				std::cerr << "Failed to get vertex position from line \"" << line << "\"!"<< std::endl;
				return false;
			}

			vertexPositions.push_back({ x, y, z });
		}
		else if (dataType == "vt")
		{ // Vertex Texture coordinate
			float u, v;
			if (!(segments >> u >> v))
			{
				std::cerr << "Failed to get texture coordinate from line \"" << line << "\"!"<< std::endl;
				return false;
			}

			vertexTexCoords.push_back({ u, v });
		}
		else if (dataType == "vn")
		{ // Normal
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				std::cerr << "Failed to get normal from line \"" << line << "\"!"<< std::endl;
				return false;
			}

			vertexNormals.push_back({ x, y, z });
		}
		else if (dataType == "f")
		{ // Index group
			for (int i = 0; i < 3; i++)
			{
				int vi, ti, ni;
				if (!(segments >> vi >> ti >> ni))
				{
					std::cerr << "Failed to get index group " << i << " from line \"" << line << "\"!"<< std::endl;
					return false;
				}

				indexGroups.push_back({ vi, ti, ni });
			}
		}
		else if (dataType == "s")
		{ // Smooth shading

		}
		else if (dataType == "usemtl")
		{ // Start of submesh with material
			if (!begunReadingSubMesh)
			{ // First submesh
				begunReadingSubMesh = true;
				continue;
			}

			// Flush previous submesh
			if (indexGroups.empty())
				continue; // Skip submesh without faces

			int indexCount = indexGroups.size();

			meshData.subMeshInfo.push_back({});
			meshData.subMeshInfo.back().startIndexValue = endOfLastSubMesh;
			meshData.subMeshInfo.back().nrOfIndicesInSubMesh = indexCount - endOfLastSubMesh;
			meshData.subMeshInfo.back().ambientTextureSRV = nullptr;
			meshData.subMeshInfo.back().diffuseTextureSRV = nullptr;
			meshData.subMeshInfo.back().specularTextureSRV = nullptr;

			endOfLastSubMesh = indexCount;
		}
		else
		{
			std::cerr << "Unrecognized type flag '" << dataType << "'!";
		}
	}

	meshData.vertexInfo.nrOfVerticesInBuffer = vertexPositions.size();
	meshData.vertexInfo.sizeOfVertex = sizeof(RawVertex);
	meshData.vertexInfo.vertexData = new RawVertex[vertexPositions.size()];
	std::memcpy(meshData.vertexInfo.vertexData, vertexPositions.data(), vertexPositions.size() * sizeof(RawVertex));

	meshData.indexInfo.nrOfIndicesInBuffer = indexGroups.size();
	meshData.indexInfo.indexData = new uint32_t[indexGroups.size()];
	for (int i = 0; i < indexGroups.size(); i++)
	{ meshData.indexInfo.indexData[i] = indexGroups.at(i).v; }

	return true;	
}