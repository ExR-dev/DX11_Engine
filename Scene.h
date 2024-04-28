#pragma once

#include <d3d11.h>

#include "SceneHolder.h"
#include "Entity.h"
#include "Graphics.h"
#include "Input.h"
#include "CameraD3D11.h"
#include "SpotLightCollectionD3D11.h"
#include "DirLightCollectionD3D11.h"
#include "PointLightCollectionD3D11.h"
#include "Cubemap.h"

// Contains and manages entities, cameras and lights. Also handles queueing entities for rendering.
class Scene
{
private:
	bool _initialized = false;
	ID3D11Device *_device = nullptr;
	Content *_content = nullptr;
	Graphics *_graphics = nullptr;

	SceneHolder _sceneHolder;

	CameraD3D11 *_camera = nullptr, *_secondaryCamera = nullptr;
	SpotLightCollectionD3D11 *_spotlights;
	DirLightCollectionD3D11 *_dirlights;
	PointLightCollectionD3D11 *_pointlights;

	Cubemap _cubemap;

	int _currCamera = -2;
	CameraD3D11 *_currCameraPtr = nullptr;

	bool _doMultiThread = true;


	[[nodiscard]] bool UpdateEntities(ID3D11DeviceContext *context, Time &time, const Input &input);

	void UpdateSelectionMarker(int i) const;

	void DebugGenerateVolumeTreeStructure();

public:
	Scene();
	~Scene();
	Scene(const Scene &other) = delete;
	Scene &operator=(const Scene &other) = delete;
	Scene(Scene &&other) = delete;
	Scene &operator=(Scene &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, Content *content, Graphics *graphics);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input);

	[[nodiscard]] bool Render(Time &time, const Input &input);
	[[nodiscard]] bool RenderUI();
};
