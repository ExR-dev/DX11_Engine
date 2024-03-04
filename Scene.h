#pragma once

#include <vector>
#include <d3d11.h>

#include "SceneHolder.h"
#include "Entity.h"
#include "Input.h"
#include "CameraD3D11.h"
#include "SpotLightCollectionD3D11.h"
#include "PointLightCollectionD3D11.h"
#include "Cubemap.h"


class Scene
{
private:
	bool _initialized;
	ID3D11Device *_device = nullptr;
	Content *_content = nullptr;

	SceneHolder _sceneHolder;

	CameraD3D11 *_camera = nullptr;
	SpotLightCollectionD3D11 *_spotLights;
	PointLightCollectionD3D11 *_pointLights;

	Cubemap _cubemap; // TODO: Discard

	int _currCamera = -1;
	CameraD3D11 *_currCameraPtr = nullptr;


public:
	Scene();
	~Scene();
	Scene(const Scene &other) = delete;
	Scene &operator=(const Scene &other) = delete;
	Scene(Scene &&other) = delete;
	Scene &operator=(Scene &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, Content *content);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, const Time &time, const Input &input);
	[[nodiscard]] bool Render(Graphics *graphics, const Time &time, const Input &input);
	[[nodiscard]] bool RenderUI() const;
};
