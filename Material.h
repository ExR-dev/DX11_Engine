#pragma once

#include "Content.h"
// TODO: Implement this instead of assigning IDs separately.

// Managed by the Content class. This will allow draw batching to be done much more cheaply.
struct Material 
{
public:
	UINT
		textureID = CONTENT_NULL,
		normalID = CONTENT_NULL,
		specularID = CONTENT_NULL,
		reflectiveID = CONTENT_NULL,
		ambientID = CONTENT_NULL,
		heightID = CONTENT_NULL;
};