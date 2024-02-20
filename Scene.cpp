#include "Scene.h"

#include <stdlib.h>
#include <cmath>

#include "ErrMsg.h"


Scene::Scene()
{
	_initialized = false;

	for (int i = 0; i < 3; i++)
	{
		_debugObjects.push_back({ });
	}
}

Scene::~Scene()
{
	if (!_initialized)
		return;

}

bool Scene::Initialize(ID3D11Device *device, Content *content)
{
	if (_initialized)
		return false;

	_camera.Initialize(
		device,
		{ 60.0f, 1.0f, 0.1f, 10.0f }
	);

	for (int i = 0; i < _debugObjects.size(); i++)
	{
		DebugObject *dObj = &_debugObjects.at(i);

		if (!dObj->Initialize(device, i, 0, 0, 0))
		{
			ErrMsg("Failed to initialize debug object!");
			return false;
		}
	}

	_initialized = true;
	return true;
}

bool Scene::Uninitialize()
{
	if (!_initialized)
		return false;

	_initialized = false;
	return true;
}


bool Scene::Update(ID3D11DeviceContext *context, const Time &time)
{
	if (!_initialized)
		return false;

	XMVECTOR camPos = { 0.0f, 0.0f, 0.0f };
	XMVECTOR camDir = { 0.0f, 0.0f, 1.0f };

	for (int i = 0; i < _debugObjects.size(); i++)
	{
		DebugObject *dObj = &_debugObjects.at(i);

		srand(8146426 + i*10000 + 0*100);
		const float rX = 0.75f * time.time * ((float)(rand() % 200) / 100.0f - 1.0f);
		srand(3871666 + i*10000 + 1*100);
		const float rY = 0.75f * time.time * ((float)(rand() % 200) / 100.0f - 1.0f);
		srand(1846144 + i*10000 + 2*100);
		const float rZ = 0.75f * time.time * ((float)(rand() % 200) / 100.0f - 1.0f);

		XMVECTOR objPos		= { 0.0f, 0.0f, 5.0f };
		/*XMVECTOR objRot	= { rX, rY, rZ };
		XMVECTOR objScale	= { 
			0.5f + fmodf(abs(rX), 1.0f) * 0.5f * (std::signbit(rX) ? -1 : 1),
			0.5f + fmodf(abs(rY), 1.0f) * 0.5f * (std::signbit(rY) ? -1 : 1),
			0.5f + fmodf(abs(rZ), 1.0f) * 0.5f * (std::signbit(rZ) ? -1 : 1)
		};*/

		XMVECTOR objRot		= { 0, -time.time, 0 };
		XMVECTOR objScale	= { 0.5f, 0.5f, 0.5f };

		objPos.m128_f32[0] = (float)(i - 1) * 1.25f;
		objPos.m128_f32[1] = (float)(i - 1) * 1.0f;

		if (!dObj->SetVPM(context, 50.0f, 1.0f, 0.1f, 15.0f, camPos, camDir))
		{
			ErrMsg("Failed to update debug object view projection matrix!");
			return false;
		}

		if (!dObj->SetWM(context))
		{
			ErrMsg("Failed to update debug object world matrix!");
			return false;
		}

		if (!dObj->Update(context, time))
		{
			ErrMsg("Failed to update debug object!");
			return false;
		}
	}

	return true;
}

bool Scene::Render(ID3D11DeviceContext *context, const Content &content)
{
	if (!_initialized)
		return false;

	for (int i = 0; i < _debugObjects.size(); i++)
	{
		DebugObject *dObj = &_debugObjects.at(i);

		if (!dObj->Render(context, content))
		{
			ErrMsg("Failed to render debug object!");
			return false;
		}
	}

	return true;
}