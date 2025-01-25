#pragma once


struct Material
{
private:
	typedef unsigned int UINT;

public:
	UINT textureID, normalID, specularID, reflectiveID, ambientID, heightID;

	Material() = default;
	Material(UINT texture, UINT normal, UINT specular, UINT reflective, UINT ambient, UINT height) 
		: textureID(texture), normalID(normal), specularID(specular), reflectiveID(reflective), ambientID(ambient), heightID(height) 
	{ }

	~Material() = default;
	Material(const Material &other) = default;
	Material &operator=(const Material &other) = default;
	Material(Material &&other) = default;
	Material &operator=(Material &&other) = default;


	bool operator==(const Material &other) const
	{
		if (textureID != other.textureID)
			return false;

		if (normalID != other.normalID)
			return false;

		if (specularID != other.specularID)
			return false;

		if (reflectiveID != other.reflectiveID)
			return false;

		if (ambientID != other.ambientID)
			return false;

		return heightID == other.heightID;
	}

	bool operator!=(const Material &other) const
	{
		return !(this == &other);
	}

	bool operator<(const Material &other) const
	{
		if (textureID != other.textureID)
			return textureID < other.textureID;

		if (normalID != other.normalID)
			return normalID < other.normalID;

		if (specularID != other.specularID)
			return specularID < other.specularID;

		if (reflectiveID != other.reflectiveID)
			return reflectiveID < other.reflectiveID;

		if (ambientID != other.ambientID)
			return ambientID < other.ambientID;

		return heightID < other.heightID;
	}
};