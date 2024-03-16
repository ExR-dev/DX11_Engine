#include "ContentLoader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "ErrMsg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct RawPosition	{ float	x, y, z; };
struct RawNormal	{ float	x, y, z; };
struct RawTexCoord	{ float	u, v;	 };
struct RawIndex		{ int	v, t, n; };

struct FormattedVertex {
	float 
		px, py, pz,
		nx, ny, nz,
		tx, ty, tz,
		u, v;

	FormattedVertex() :
		px(0.0f), py(0.0f), pz(0.0f),
		nx(0.0f), ny(0.0f), nz(0.0f),
		tx(0.0f), ty(0.0f), tz(0.0f),
		u(0.0f), v(0.0f)
	{ }

	FormattedVertex(
		const float px, const float py, const float pz, 
		const float nx, const float ny, const float nz, 
		const float tx, const float ty, const float tz, 
		const float u, const float v) :
		px(px), py(py), pz(pz),
		nx(nx), ny(ny), nz(nz),
		tx(tx), ty(ty), tz(tz),
		u(u), v(v)
	{ }

	bool operator==(const FormattedVertex &other) const
	{
		if (px != other.px) return false;
		if (py != other.py) return false;
		if (pz != other.pz) return false;

		if (nx != other.nx) return false;
		if (ny != other.ny) return false;
		if (nz != other.nz) return false;

		if (tx != other.tx) return false;
		if (ty != other.ty) return false;
		if (tz != other.tz) return false;

		if (u != other.u) return false;
		if (v != other.v) return false;

		return true;
	}
};


static bool ReadWavefront(const char *path, 
	std::vector<RawPosition> &vertexPositions, 
	std::vector<RawTexCoord> &vertexTexCoords,
	std::vector<RawNormal> &vertexNormals,
	std::vector<std::vector<RawIndex>> &indexGroups)
{
	std::ifstream fileStream(path);
	std::string line;

	bool begunReadingSubMesh = false;

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
			ErrMsg(std::format("Failed to get data type from line \"{}\", file \"{}\"!", line, path));
			return false;
		}

		if (dataType == "mtllib")
		{ // Define where to find materials

		}
		else if (dataType == "g")
		{ // Mesh Group
			if (!begunReadingSubMesh)
			{ // First submesh
				begunReadingSubMesh = true;
				indexGroups.emplace_back();
				continue;
			}

			indexGroups.emplace_back();
		}
		else if (dataType == "o")
		{ // Mesh Object
			if (!begunReadingSubMesh)
			{ // First submesh
				begunReadingSubMesh = true;
				indexGroups.emplace_back();
				continue;
			}

			indexGroups.emplace_back();
		}
		else if (dataType == "v")
		{ // Vertex Position
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				ErrMsg(std::format(R"(Failed to get vertex position from line "{}", file "{}"!)", line, path));
				return false;
			}

			vertexPositions.push_back({ x, y, z });
		}
		else if (dataType == "vt")
		{ // Vertex Texture Coordinate
			float u, v;
			if (!(segments >> u >> v))
			{
				ErrMsg(std::format(R"(Failed to get texture coordinate from line "{}", file "{}"!)", line, path));
				return false;
			}

			vertexTexCoords.push_back({ u, v });
		}
		else if (dataType == "vn")
		{ // Vertex Normal
			float x, y, z;
			if (!(segments >> x >> y >> z))
			{
				ErrMsg(std::format(R"(Failed to get normal from line "{}", file "{}"!)", line, path));
				return false;
			}

			vertexNormals.push_back({ x, y, z });
		}
		else if (dataType == "f")
		{ // Index Group
			if (!begunReadingSubMesh)
			{
				ErrMsg(std::format(R"(Reached index group before creating submesh, file "{}"!)", path));
				return false;
			}

			std::vector<RawIndex> indicesInGroup;
			int vi, ti, ni;

			while (segments >> vi >> ti >> ni)
				indicesInGroup.push_back({ --vi, --ti, --ni });

			size_t groupSize = indicesInGroup.size();
			if (groupSize < 3 || groupSize > 4)
			{
				ErrMsg(std::format(R"(Unparseable group size '{}' at line "{}", file "{}"!)", groupSize, line, path));
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
			ErrMsg(std::format(R"(Unimplemented type flag '{}' on line "{}", file "{}"!)", dataType, line, path));
		}
	}
	return true;
}

static void FormatRawMesh(
	std::vector<FormattedVertex> &formattedVertices,
	std::vector<std::vector<uint32_t>> &formattedIndexGroups,
	const std::vector<RawPosition> &vertexPositions,
	const std::vector<RawTexCoord> &vertexTexCoords,
	const std::vector<RawNormal> &vertexNormals,
	const std::vector<std::vector<RawIndex>> &indexGroups)
{
	// Format vertices & index groups
	const size_t groupCount = indexGroups.size();
	for (size_t groupIndex = 0; groupIndex < groupCount; groupIndex++)
	{
		formattedIndexGroups.emplace_back();
		std::vector<uint32_t> *formattedGroup = &formattedIndexGroups.back();
		const std::vector<RawIndex> *rawGroup = &indexGroups.at(groupIndex);

		const size_t groupSize = rawGroup->size();
		for (size_t vertIndex = 0; vertIndex < groupSize; vertIndex++)
		{
			const RawIndex rI = rawGroup->at(vertIndex);
			const RawPosition rP = vertexPositions.at(rI.v);
			const RawTexCoord rT = vertexTexCoords.at(rI.t);
			const RawNormal rN = vertexNormals.at(rI.n);

			formattedGroup->emplace_back(static_cast<uint32_t>(formattedVertices.size()));
			formattedVertices.emplace_back(
				rP.x, rP.y, rP.z,
				rN.x, rN.y, rN.z,
				0,	  0,    0,
				rT.u, rT.v
			);
		}

		// Generate tangents
		for (size_t triIndex = 0; triIndex < groupSize; triIndex += 3)
		{
			FormattedVertex *verts[3] = {
			&formattedVertices.at(triIndex + 0),
			&formattedVertices.at(triIndex + 1),
			&formattedVertices.at(triIndex + 2)
			};

			const DirectX::XMFLOAT3A
				v0 = { verts[0]->px, verts[0]->py, verts[0]->pz },
				v1 = { verts[1]->px, verts[1]->py, verts[1]->pz },
				v2 = { verts[2]->px, verts[2]->py, verts[2]->pz };

			const DirectX::XMFLOAT3A
				edge1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z },
				edge2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

			const DirectX::XMFLOAT2A
				uv0 = { verts[0]->u, verts[0]->v },
				uv1 = { verts[1]->u, verts[1]->v },
				uv2 = { verts[2]->u, verts[2]->v };

			const DirectX::XMFLOAT2A
				deltaUV1 = { uv1.x - uv0.x,	uv1.y - uv0.y },
				deltaUV2 = { uv2.x - uv0.x,	uv2.y - uv0.y };

			const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			DirectX::XMFLOAT4A tangent = {
				f * (edge1.x * deltaUV2.y - edge2.x * deltaUV1.y),
				f * (edge1.y * deltaUV2.y - edge2.y * deltaUV1.y),
				f * (edge1.z * deltaUV2.y - edge2.z * deltaUV1.y),
				0.0f
			};

			for (size_t i = 0; i < 3; i++)
			{
				// Gram-Schmidt orthogonalization
				const DirectX::XMFLOAT4A normal = { verts[i]->nx, verts[i]->ny, verts[i]->nz, 0.0f };

				const DirectX::XMVECTOR
					n = *reinterpret_cast<const DirectX::XMVECTOR *>(&normal),
					t = *reinterpret_cast<const DirectX::XMVECTOR *>(&tangent);

				const DirectX::XMVECTOR newTangentVec = DirectX::XMVector3Normalize(
					DirectX::XMVectorSubtract(t,
						DirectX::XMVectorScale(n,
							DirectX::XMVectorGetX(
								DirectX::XMVector3Dot(n, t)))));

				const DirectX::XMFLOAT4A newTangent = *reinterpret_cast<const DirectX::XMFLOAT4A *>(&newTangentVec);

				verts[i]->tx = newTangent.x;
				verts[i]->ty = newTangent.y;
				verts[i]->tz = newTangent.z;
			}
		}
	}
}

static void SendFormattedMeshToMeshData(MeshData &meshData,
	const std::vector<FormattedVertex> &formattedVertices,
	const std::vector<std::vector<uint32_t>> &formattedIndexGroups)
{
	// Send vertex data to meshData
	meshData.vertexInfo.nrOfVerticesInBuffer = formattedVertices.size();
	meshData.vertexInfo.sizeOfVertex = sizeof(FormattedVertex);
	meshData.vertexInfo.vertexData = reinterpret_cast<float *>(new FormattedVertex[meshData.vertexInfo.nrOfVerticesInBuffer]);

	std::memcpy(
		meshData.vertexInfo.vertexData,
		formattedVertices.data(),
		meshData.vertexInfo.sizeOfVertex * meshData.vertexInfo.nrOfVerticesInBuffer
	);

	// Send index data to meshData
	std::vector<uint32_t> inlineIndices;

	const size_t groupCount = formattedIndexGroups.size();
	for (size_t group_i = 0; group_i < groupCount; group_i++)
	{
		MeshData::SubMeshInfo subMeshInfo = { };
		const std::vector<uint32_t> *currGroup = &formattedIndexGroups.at(group_i);

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

	// Calculate bounds
	DirectX::XMFLOAT4A
		min = {  FLT_MAX,  FLT_MAX,  FLT_MAX, 0 },
		max = { -FLT_MAX, -FLT_MAX, -FLT_MAX, 0 };

	for (const FormattedVertex &vData : formattedVertices)
	{
		if (vData.px < min.x)		min.x = vData.px;
		else if (vData.px > max.x)	max.x = vData.px;

		if (vData.py < min.y)		min.y = vData.py;
		else if (vData.py > max.y)	max.y = vData.py;

		if (vData.pz < min.z)		min.z = vData.pz;
		else if (vData.pz > max.z)	max.z = vData.pz;
	}

	DirectX::BoundingBox().CreateFromPoints(
		meshData.boundingBox,
		*reinterpret_cast<DirectX::XMVECTOR *>(&min),
		*reinterpret_cast<DirectX::XMVECTOR *>(&max)
	);
}


bool LoadMeshFromFile(const char *path, MeshData &meshData)
{
	if (meshData.vertexInfo.vertexData != nullptr || 
		meshData.indexInfo.indexData != nullptr)
	{
		ErrMsg("meshData is not nullified!");
		return false;
	}

	std::string ext = path;
	ext.erase(0, ext.find_last_of('.') + 1);

	std::vector<RawPosition> vertexPositions;
	std::vector<RawTexCoord> vertexTexCoords;
	std::vector<RawNormal> vertexNormals;
	std::vector<std::vector<RawIndex>> indexGroups;

	if (ext == "obj")
	{
		if (!ReadWavefront(path, vertexPositions, vertexTexCoords, vertexNormals, indexGroups))
		{
			ErrMsg("Failed to read wavefront file!");
			return false;
		}
	}
	else
	{
		ErrMsg(std::format("Unimplemented mesh file extension '{}'!", ext));
		return false;
	}

	std::vector<FormattedVertex> formattedVertices;
	std::vector<std::vector<uint32_t>> formattedIndexGroups;

	FormatRawMesh(formattedVertices, formattedIndexGroups, vertexPositions, vertexTexCoords, vertexNormals, indexGroups);

	SendFormattedMeshToMeshData(meshData, formattedVertices, formattedIndexGroups);

	return true;	
}

/// Debug Function
bool WriteMeshToFile(const char *path, const MeshData &meshData)
{
	if (meshData.vertexInfo.vertexData == nullptr || 
		meshData.indexInfo.indexData == nullptr)
	{
		ErrMsg("meshData is nullified!");
		return false;
	}

	std::ofstream fileStream(path);

	fileStream << "Loaded Mesh:\n\n";

	fileStream << "---------------- Vertex Data ----------------" << '\n';
	fileStream << "count = " << meshData.vertexInfo.nrOfVerticesInBuffer << '\n';
	fileStream << "size = " << meshData.vertexInfo.sizeOfVertex << "\n\n";

	for (size_t i = 0; i < meshData.vertexInfo.nrOfVerticesInBuffer; i++)
	{
		const FormattedVertex *vData = &reinterpret_cast<FormattedVertex*>(meshData.vertexInfo.vertexData)[i];

		fileStream << "Vertex " << i << '\n';

		fileStream << "\tPosition(" << vData->px << ", " << vData->py << ", " << vData->pz << ")\n";
		fileStream << "\tNormal(" << vData->nx << ", " << vData->ny << ", " << vData->nz << ")\n";
		fileStream << "\tTangent(" << vData->tx << ", " << vData->ty << ", " << vData->tz << ")\n";
		fileStream << "\tTexCoord(" << vData->u << ", " << vData->v << ")\n";

		fileStream << '\n';
	}
	fileStream << "---------------------------------------------\n\n";

	fileStream << "---------------- Index Data ----------------\n";
	fileStream << "count = " << meshData.indexInfo.nrOfIndicesInBuffer << '\n';

	for (size_t i = 0; i < meshData.indexInfo.nrOfIndicesInBuffer / 3; i++)
	{
		const uint32_t *iData = &meshData.indexInfo.indexData[i*3];

		fileStream << "indices " << i*3 << "-" << i*3+2 << "\t (" << iData[0] << "/" << iData[1] << "/" << iData[2] << ")\n";
	}
	fileStream << "--------------------------------------------\n\n";

	fileStream << "---------------- Triangle Data ----------------\n";
	fileStream << "count = " << meshData.indexInfo.nrOfIndicesInBuffer / 3 << '\n';

	for (size_t i = 0; i < meshData.indexInfo.nrOfIndicesInBuffer / 3; i++)
	{
		const uint32_t *iData = &meshData.indexInfo.indexData[i*3];

		fileStream << "Triangle " << i+1 << " {";

		for (size_t j = 0; j < 3; j++)
		{
			const FormattedVertex *vData = &reinterpret_cast<FormattedVertex *>(meshData.vertexInfo.vertexData)[iData[j]];
			fileStream << "\n\tv" << j << '\n';

			fileStream << "\t\tP Vector3(" << vData->px << ", " << vData->py << ", " << vData->pz << ")\n";
			fileStream << "\t\tN Vector3(" << vData->nx << ", " << vData->ny << ", " << vData->nz << ")\n";
			fileStream << "\t\tT Vector3(" << vData->tx << ", " << vData->ty << ", " << vData->tz << ")\n";
			fileStream << "\t\tu Vector3(" << vData->u << ", " << vData->v << ", 0)\n";
		}

		fileStream << "}\n\n";
	}
	fileStream << "--------------------------------------------\n\n\n";

	fileStream << "---------------- Submesh Data ----------------\n";
	for (size_t i = 0; i < meshData.subMeshInfo.size(); i++)
	{
		fileStream << "Submesh " << i << '\n';
		fileStream << "\tstart index = " << meshData.subMeshInfo.at(i).startIndexValue << '\n';
		fileStream << "\tlength = " << meshData.subMeshInfo.at(i).nrOfIndicesInSubMesh << '\n';
		fileStream << '\n';
	}
	fileStream << "----------------------------------------------\n\n";
	
	fileStream.close();
	return true;
}


bool LoadTextureFromFile(const char *path, UINT &width, UINT &height, std::vector<unsigned char> &data)
{
	stbi_set_flip_vertically_on_load(1);

	int w, h, comp;
	unsigned char *imgData = stbi_load(path, &w, &h, &comp, STBI_rgb_alpha);
	if (imgData == nullptr)
	{
		ErrMsg(std::format("Failed to load texture from file at path \"{}\"!", path));
		return false;
	}

	width = static_cast<UINT>(w);
	height = static_cast<UINT>(h);
	data = std::vector(imgData, imgData + static_cast<size_t>(4ul * w * h));

	stbi_image_free(imgData);
	return true;
}
