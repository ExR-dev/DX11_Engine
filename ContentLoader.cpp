#include "ContentLoader.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>


struct RawVertex	{ float		x, y, z;	};
struct RawNormal	{ float		x, y, z;	};
struct RawTexCoord	{ float		u, v;		};
struct RawIndex		{ int		v, t, n;	};


bool LoadMeshFromFile(const char *path, MeshData &meshData)
{
	std::ifstream fileStream(path);
	std::string line;

	std::vector<RawVertex>		currVertices;
	std::vector<RawNormal>		currNormals;
	std::vector<RawTexCoord>	currTexCoords;
	std::vector<RawIndex>		currIndices;
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
		{ // Vertex

		}
		else if (dataType == "vt")
		{ // Texture coordinate

		}
		else if (dataType == "vn")
		{ // Normal

		}
		else if (dataType == "f")
		{ // Index group

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
			if (currIndices.empty())
				continue; // Skip submesh without faces

			// Clear submesh data
			currIndices.clear();
		}
		else
		{
			std::cerr << "Unrecognized type flag '" << dataType << "'!";
		}
	}

	return true;
}