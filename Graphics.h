#pragma once

#include "Entity.h"


class Graphics
{
private:
	bool _isRendering = false;

public:
	Graphics();
	~Graphics();


	int BeginRender();
	int EndRender();

	int Render(const Entity &entity);
};
