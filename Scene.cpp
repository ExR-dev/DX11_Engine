#include "Scene.h"

#include <iostream>
#include <stdlib.h>
#include <cmath>


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

bool Scene::Initialize(ID3D11Device *device)
{
	if (_initialized)
		return false;

	for (int i = 0; i < _debugObjects.size(); i++)
	{
		DebugObject *dObj = &_debugObjects.at(i);

		if (!dObj->Initialize(device))
		{
			std::cerr << "Failed to initialize debug object!" << std::endl;
			OutputDebugString(L"Failed to initialize debug object!\n");
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


	for (int i = 0; i < _debugObjects.size(); i++)
	{
		DebugObject *dObj = &_debugObjects.at(i);

		if (!dObj->Uninitialize())
		{
			std::cerr << "Failed to uninitialize debug object!" << std::endl;
			OutputDebugString(L"Failed to uninitialize debug object!\n");
			return false;
		}
	}

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
		float rX = 0.75f * time.time * ((float)(rand() % 200) / 100.0f - 1.0f);
		srand(3871666 + i*10000 + 1*100);
		float rY = 0.75f * time.time * ((float)(rand() % 200) / 100.0f - 1.0f);
		srand(1846144 + i*10000 + 2*100);
		float rZ = 0.75f * time.time * ((float)(rand() % 200) / 100.0f - 1.0f);

		XMVECTOR objPos	= { 0.0f, 0.0f, 5.0f };
		XMVECTOR objRot	= { rX, rY, rZ }; // = { 0.0f, 0.0f, 0.0f };
		XMVECTOR objScale = { 
			1 + fmodf(abs(rX), 1.0f) * (std::signbit(rX) ? -1 : 1), 
			1 + fmodf(abs(rY), 1.0f) * (std::signbit(rY) ? -1 : 1),
			1 + fmodf(abs(rZ), 1.0f) * (std::signbit(rZ) ? -1 : 1) 
		};

		objPos.m128_f32[0] = (float)(i - 1) * 1.25f;
		//objRot.m128_f32[1] = time.time * (float)i;

		if (!dObj->SetVPM(context, 50.0f, 1.0f, 0.1f, 15.0f, camPos, camDir))
		{
			std::cerr << "Failed to update debug object view projection matrix!" << std::endl;
			OutputDebugString(L"Failed to update debug object view projection matrix!\n");
			return false;
		}

		if (!dObj->SetWM(context, objPos, objRot, objScale))
		{
			std::cerr << "Failed to update debug object world matrix!" << std::endl;
			OutputDebugString(L"Failed to update debug object world matrix!\n");
			return false;
		}

		if (!dObj->Update(context, time))
		{
			std::cerr << "Failed to update debug object!" << std::endl;
			OutputDebugString(L"Failed to update debug object!\n");
			return false;
		}
	}

	return true;
}

bool Scene::Render(ID3D11DeviceContext *context)
{
	if (!_initialized)
		return false;

	for (int i = 0; i < _debugObjects.size(); i++)
	{
		DebugObject *dObj = &_debugObjects.at(i);

		if (!dObj->Render(context))
		{
			std::cerr << "Failed to render debug object!" << std::endl;
			OutputDebugString(L"Failed to render debug object!\n");
			return false;
		}
	}

	return true;
}