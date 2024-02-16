#include "ContentLoader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "ErrMsg.h"


struct RawPosition	{ float	x, y, z; };
struct RawNormal	{ float	x, y, z; };
struct RawTexCoord	{ float	u, v;	 };
struct RawIndex		{ int	v, t, n; };


struct FormattedVertex {
	float 
		px, py, pz,
		nx, ny, nz,
		u, v;

	bool operator==(const FormattedVertex &other) const
	{
		if (px != other.px) return false;
		if (py != other.py) return false;
		if (pz != other.pz) return false;

		if (nx != other.nx) return false;
		if (ny != other.ny) return false;
		if (nz != other.nz) return false;

		if (u != other.u) return false;
		if (v != other.v) return false;

		return true;
	}
};


void ResolveDuplicateVertices(
	std::vector<FormattedVertex> &vertices,
	std::vector<std::vector<uint32_t>> &indexGroups)
{
	const auto vertBegin = vertices.begin();
	int vertCount = vertices.size();

	std::vector<uint32_t> vertexMap(vertCount, 0);
	for (size_t i = 0; i < vertCount; (vertexMap.at(i) = i++));

	// Locate and erase duplicates
	for (size_t i = 0; i < vertCount; i++)
	{
		const FormattedVertex checkedVert = vertices.at(i);

		for (size_t j = i + 1; j < vertCount; j++)
		{
			if (vertices.at(j) == checkedVert)
			{
				vertexMap.at(j) = i;
				vertices.erase(vertBegin + j);

				vertCount--;
				j--;
			}
		}
	}

	const int groupCount = indexGroups.size();

	// Remap indices
	for (size_t group_i = 0; group_i < groupCount; group_i++)
	{
		std::vector<uint32_t> *currGroup = &indexGroups.at(group_i);
		const int groupSize = currGroup->size();

		for (size_t i = 0; i < groupSize; i++)
		{
			const uint32_t oldIndex = currGroup->at(i);
			currGroup->at(i) = vertexMap.at(oldIndex);
		}
	}
}


bool LoadMeshFromFile(const char *path, MeshData &meshData)
{
	if (meshData.vertexInfo.vertexData != nullptr || 
		meshData.indexInfo.indexData != nullptr)
	{
		ErrMsg("meshData is not nullified!");
		return false;
	}

	std::ifstream fileStream(path);
	std::string line;

	std::vector<RawPosition> vertexPositions;
	std::vector<RawTexCoord> vertexTexCoords;
	std::vector<RawNormal> vertexNormals;
	std::vector<std::vector<RawIndex>> indexGroups;

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
			ErrMsg("Failed to get data type from line \"" + line + "\"!");
			return false;
		}

		if (dataType == "mtllib") 
		{ // Define where to find materials

		}
		else if (dataType == "g")
		{ // Mesh Group
			
		}
		else if (dataType == "o")
		{ // Mesh Object
			
		}
		else if (dataType == "v")
		{ // Vertex Position
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				ErrMsg("Failed to get vertex position from line \"" + line + "\"!");
				return false;
			}

			vertexPositions.push_back({ x, y, z });
		}
		else if (dataType == "vt")
		{ // Vertex Texture Coordinate
			float u, v;
			if (!(segments >> u >> v))
			{
				ErrMsg("Failed to get texture coordinate from line \"" + line + "\"!");
				return false;
			}

			vertexTexCoords.push_back({ u, v });
		}
		else if (dataType == "vn")
		{ // Vertex Normal
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				ErrMsg("Failed to get normal from line \"" + line + "\"!");
				return false;
			}

			vertexNormals.push_back({ x, y, z });
		}
		else if (dataType == "f")
		{ // Index Group
			if (!begunReadingSubMesh)
			{
				ErrMsg("Reached index group before creating submesh!");
				return false;
			}

			std::vector<RawIndex> indicesInGroup;
			int vi, ti, ni;

			while (segments >> vi >> ti >> ni)
				indicesInGroup.push_back({ --vi, --ti, --ni });

			int groupSize = indicesInGroup.size();
			if (groupSize < 3 || groupSize > 4)
			{
				ErrMsg("Unparseable group size '" + std::to_string(groupSize) + "' at line \"" + line + "\"!");
				return false;
			}

			indexGroups.back().push_back(indicesInGroup.at(0));
			indexGroups.back().push_back(indicesInGroup.at(1));
			indexGroups.back().push_back(indicesInGroup.at(2));

			if (groupSize == 4)
			{ // Group is a quad
				indexGroups.back().push_back(indicesInGroup.at(0));
				indexGroups.back().push_back(indicesInGroup.at(2));
				indexGroups.back().push_back(indicesInGroup.at(3));
			}
		}
		else if (dataType == "usemtl")
		{ // Start of submesh with material
			if (!begunReadingSubMesh)
			{ // First submesh
				begunReadingSubMesh = true;
				indexGroups.emplace_back();
				continue;
			}

			indexGroups.emplace_back();
		}
		else
		{
			ErrMsg("Unrecognized type flag '" + dataType + "'!");
		}
	}

	std::vector<FormattedVertex> formattedVertices;
	std::vector<std::vector<uint32_t>> formattedIndexGroups;

	size_t groupCount = indexGroups.size();
	for (size_t groupIndex = 0; groupIndex < groupCount; groupIndex++)
	{
		formattedIndexGroups.emplace_back();
		std::vector<uint32_t> *formattedGroup = &formattedIndexGroups.back();
		const std::vector<RawIndex> *rawGroup = &indexGroups.at(groupIndex);

		size_t groupSize = rawGroup->size();
		for (size_t vertIndex = 0; vertIndex < groupSize; vertIndex++)
		{
			RawIndex rI = rawGroup->at(vertIndex);

			RawPosition rP = vertexPositions.at(rI.v);
			RawTexCoord rT = vertexTexCoords.at(rI.t);
			RawNormal rN = vertexNormals.at(rI.n);

			FormattedVertex newVert = { 
				rP.x, rP.y, rP.z,
				rN.x, rN.y, rN.z,
				rT.u, rT.v,				
			};

			formattedGroup->push_back(formattedVertices.size());
			formattedVertices.push_back(newVert);
		}
	}

	/* Keep or discard ? */
	// ResolveDuplicateVertices(formattedVertices, formattedIndexGroups); 

	// Send vertex data to meshData
	meshData.vertexInfo.nrOfVerticesInBuffer = formattedVertices.size();
	meshData.vertexInfo.sizeOfVertex = sizeof(FormattedVertex);
	meshData.vertexInfo.vertexData = reinterpret_cast<float*>(new FormattedVertex[meshData.vertexInfo.nrOfVerticesInBuffer]);

	std::memcpy(
		meshData.vertexInfo.vertexData,
		formattedVertices.data(),
		meshData.vertexInfo.sizeOfVertex * meshData.vertexInfo.nrOfVerticesInBuffer
	);

	// Send index data to meshData
	std::vector<uint32_t> inlineIndices;

	groupCount = formattedIndexGroups.size();
	for (size_t group_i = 0; group_i < groupCount; group_i++)
	{
		MeshData::SubMeshInfo subMeshInfo = { };
		std::vector<uint32_t> *currGroup = &formattedIndexGroups.at(group_i);

		subMeshInfo.startIndexValue = inlineIndices.size();
		subMeshInfo.nrOfIndicesInSubMesh = currGroup->size();

		inlineIndices.insert(inlineIndices.end(), currGroup->begin(), currGroup->end());
		meshData.subMeshInfo.push_back(subMeshInfo);
	}

	meshData.indexInfo.nrOfIndicesInBuffer = inlineIndices.size();
	meshData.indexInfo.indexData = new uint32_t[meshData.indexInfo.nrOfIndicesInBuffer];

	std::memcpy(
		meshData.indexInfo.indexData,
		inlineIndices.data(),
		sizeof(uint32_t) * meshData.indexInfo.nrOfIndicesInBuffer
	);

	return true;	
}

/// Debug Function
bool WriteMeshToFile(const char *path, MeshData &meshData)
{
	if (meshData.vertexInfo.vertexData == nullptr || 
		meshData.indexInfo.indexData == nullptr)
	{
		ErrMsg("meshData is nullified!");
		return false;
	}

	std::ofstream fileStream(path);

	fileStream << "Loaded Mesh:" << std::endl << std::endl;

	fileStream << "---------------- Vertex Data ----------------" << std::endl;
	fileStream << "count = " << meshData.vertexInfo.nrOfVerticesInBuffer << std::endl;
	fileStream << "size = " << meshData.vertexInfo.sizeOfVertex << std::endl << std::endl;

	for (size_t i = 0; i < meshData.vertexInfo.nrOfVerticesInBuffer; i++)
	{
		const FormattedVertex *vData = &reinterpret_cast<FormattedVertex*>(meshData.vertexInfo.vertexData)[i];

		fileStream << "Vertex " << i << std::endl;

		fileStream << "\tPosition(" << vData->px << ", " << vData->py << ", " << vData->pz << ")" << std::endl;
		fileStream << "\tNormal(" << vData->nx << ", " << vData->ny << ", " << vData->nz << ")" << std::endl;
		fileStream << "\tTexCoord(" << vData->u << ", " << vData->v << ")" << std::endl;

		fileStream << std::endl;
	}
	fileStream << "---------------------------------------------" << std::endl << std::endl;

	fileStream << "---------------- Index Data ----------------" << std::endl;
	fileStream << "count = " << meshData.indexInfo.nrOfIndicesInBuffer << std::endl;

	for (size_t i = 0; i < meshData.indexInfo.nrOfIndicesInBuffer / 3; i++)
	{
		const uint32_t *iData = &meshData.indexInfo.indexData[i*3];

		fileStream << "indices " << i*3 << "-" << i*3+2 << "\t (" << iData[0] << "/" << iData[1] << "/" << iData[2] << ")" << std::endl;
	}
	fileStream << "--------------------------------------------" << std::endl << std::endl << std::endl;

	fileStream << "---------------- Submesh Data ----------------" << std::endl;
	for (size_t i = 0; i < meshData.subMeshInfo.size(); i++)
	{
		fileStream << "Submesh " << i << std::endl;
		fileStream << "\tstart index = " << meshData.subMeshInfo.at(i).startIndexValue << std::endl;
		fileStream << "\tlength = " << meshData.subMeshInfo.at(i).nrOfIndicesInSubMesh << std::endl;
		fileStream << std::endl;
	}
	fileStream << "----------------------------------------------" << std::endl << std::endl;
	
	fileStream.close();
	return true;
}