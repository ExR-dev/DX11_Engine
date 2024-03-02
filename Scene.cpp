#include "Scene.h"

#include <cstdlib>

#include "ErrMsg.h"


Scene::Scene()
{
	_initialized = false;

	_camera = new CameraD3D11();
	_spotLights = new SpotLightCollectionD3D11();
	_pointLights = new PointLightCollectionD3D11();

	_currCameraPtr = _camera;
}

Scene::~Scene()
{
	const size_t entityCount = _entities.size();
	for (size_t i = 0; i < entityCount; i++)
		delete _entities.at(i).item;

	delete _pointLights;
	delete _spotLights;
	delete _camera;
}

bool Scene::Initialize(ID3D11Device *device, Content *content)
{
	if (_initialized)
		return false;

	_device = device;

	_totalMeshes = content->GetMeshCount();
	_totalTextures = content->GetTextureCount();

	if (!_camera->Initialize(device, { 75.0f * (XM_PI / 180.0f), 16.0f / 9.0f, 0.1f, 50.0f}, {0.0f, 1.5f, -1.0f, 0.0f}))
	{
		ErrMsg("Failed to initialize camera!");
		return false;
	}
	
	if (!_cubemap.Initialize(device, 0.1f, 50.0f, {0.0f, 0.0f, 0.0f, 0.0f}))
	{
		ErrMsg("Failed to initialize cubemap!");
		return false;
	}

	const SpotLightData spotLightInfo = {
		2048,
		std::vector<SpotLightData::PerLightInfo> {
			SpotLightData::PerLightInfo {
				{ 5.0f, 0.0f, 0.0f },		// color
				0.0f,						// rotationX
				0.0f,						// rotationY
				XM_PI * 0.4f,				// angle
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 2.5f, -3.25f }		// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 5.0f, 0.0f },		// color
				0.0f,						// rotationX
				0.3f,						// rotationY
				XM_PI * 0.4f,				// angle
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 3.5f, -3.25f }		// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 0.0f, 5.0f },		// color
				0.0f,						// rotationX
				-0.3f,						// rotationY
				XM_PI * 0.4f,				// angle
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 1.5f, -3.25f }		// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 0.5f, 0.5f, 0.5f },		// color
				0.0f,						// rotationX
				XM_PIDIV2,					// rotationY
				XM_PI * 0.1f,				// angle
				0.2f,						// projectionNearZ
				10.5f,						// projectionFarZ
				{ 0.0f, 10.0f, 0.0f }		// initialPosition
			},
		}
	};

	if (!_spotLights->Initialize(device, spotLightInfo))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}

	PointLightData pointLightInfo = { };
	pointLightInfo.shadowCubeMapInfo.textureDimension = 1024;
	pointLightInfo.perLightInfo.push_back({
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		0.1f,
		50.0f
	});

	if (!_pointLights->Initialize(device, pointLightInfo))
	{
		ErrMsg("Failed to initialize point light collection!");
		return false;
	}

	// Create room entity
	{
		_entities.push_back({ new Entity(static_cast<UINT>(_entities.size())) });
		const SceneEntity &ent = _entities.back();

		if (!ent.item->Initialize(_device, 0, 2, 0, 2, 2))
		{
			ErrMsg("Failed to initialize room entity!");
			return false;
		}

		ent.item->GetTransform()->ScaleRelative({ 15.0f, 15.0f, 15.0f, 0 });
	}

	// Create model
	{
		_entities.push_back({ new Entity(static_cast<UINT>(_entities.size())) });
		const SceneEntity &ent = _entities.back();

		if (!ent.item->Initialize(_device, 0, 6, 0, 2, 7))
		{
			ErrMsg("Failed to initialize model!");
			return false;
		}

		ent.item->GetTransform()->Move({ 0.0f, 0.0f, 0.0f, 0 });
		ent.item->GetTransform()->Rotate({ 0.0f, XM_PI, 0.0f, 0 });
		ent.item->GetTransform()->ScaleRelative({ 0.3f, 0.3f, 0.3f, 0 });
	}

	// Create error
	{
		_entities.push_back({ new Entity(static_cast<UINT>(_entities.size())) });
		const SceneEntity &ent = _entities.back();

		if (!ent.item->Initialize(_device, 0, 0, 0, 2, 1))
		{
			ErrMsg("Failed to initialize error!");
			return false;
		}

		ent.item->GetTransform()->Move({ -4.0f, 3.0f, 7.0f, 0 });
		ent.item->GetTransform()->Rotate({ 0.0f, -XM_PIDIV2, 0.0f, 0 });
		ent.item->GetTransform()->ScaleRelative({ 1.2f, 1.2f, 1.2f, 0 });
	}
	
	_initialized = true;
	return true;
}


bool Scene::Update(ID3D11DeviceContext *context, const Time &time, const Input &input)
{
	if (!_initialized)
		return false;


	/*_spotLights->GetLightCamera(0)->MoveRight(time.deltaTime * -1.5f);
	_spotLights->GetLightCamera(0)->LookX(time.deltaTime * 0.5f);

	_spotLights->GetLightCamera(1)->MoveRight(time.deltaTime * -1.5f * -1.4786f);
	_spotLights->GetLightCamera(1)->LookX(time.deltaTime * 0.5f * -1.4786f);

	_spotLights->GetLightCamera(2)->MoveRight(time.deltaTime * -1.5f * 1.84248f);
	_spotLights->GetLightCamera(2)->LookX(time.deltaTime * 0.5f * 1.84248f);*/


	if (input.IsCursorLocked()) // Handle user input
	{
		if (input.GetKey(KeyCode::P) == KeyState::Pressed)
		{ // Create a random entity
			for (size_t i = 0; i < 1; i++)
			{
				_entities.push_back({ new Entity(static_cast<UINT>(_entities.size())) });
				const SceneEntity &ent = _entities.back();

				if (!ent.item->Initialize(
					_device, 
					0, 
					rand() % _totalMeshes, 
					0, 
					2, 
					rand() % _totalTextures))
				{
					ErrMsg(std::format("Failed to initialize entity #{}!", _entities.size() - 1));
					return false;
				}

				ent.item->GetTransform()->Move({
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					0
				});

				ent.item->GetTransform()->Rotate({
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					0
				});
			}
		}

		static int currSelection = -1;
		for (unsigned char i = 0; i < _entities.size(); i++)
		{
			if (i > 10)
				break;

			if (input.GetKey(static_cast<KeyCode>(static_cast<unsigned char>(KeyCode::D1) + i)) == KeyState::Pressed)
			{
				currSelection = (currSelection == i) ? -1 : i;
				break;
			}
		}

		if (input.GetKey(KeyCode::Add) == KeyState::Pressed)
			currSelection++;
		if (input.GetKey(KeyCode::Subtract) == KeyState::Pressed)
			currSelection--;

		if (currSelection == -1)
		{ // Move camera
			if (input.GetKey(KeyCode::D) == KeyState::Held)
				_currCameraPtr->MoveRight(time.deltaTime * 3.5f);
			else if (input.GetKey(KeyCode::A) == KeyState::Held)
				_currCameraPtr->MoveRight(-time.deltaTime * 3.5f);

			if (input.GetKey(KeyCode::Space) == KeyState::Held)
				_currCameraPtr->MoveUp(time.deltaTime * 3.5f);
			else if (input.GetKey(KeyCode::X) == KeyState::Held)
				_currCameraPtr->MoveUp(-time.deltaTime * 3.5f);

			if (input.GetKey(KeyCode::W) == KeyState::Held)
				_currCameraPtr->MoveForward(time.deltaTime * 3.5f);
			else if (input.GetKey(KeyCode::S) == KeyState::Held)
				_currCameraPtr->MoveForward(-time.deltaTime * 3.5f);

			const MouseState mState = input.GetMouse();
			_currCameraPtr->LookX(static_cast<float>(mState.dx) / 360.0f);
			_currCameraPtr->LookY(static_cast<float>(mState.dy) / 360.0f);
		}
		else
		{ // Move selected entity
			static bool isRotating = false;
			if (input.GetKey(KeyCode::R) == KeyState::Pressed)
				isRotating = !isRotating;
			static bool isScaling = false;
			if (input.GetKey(KeyCode::T) == KeyState::Pressed)
				isScaling = !isScaling;

			Transform *entityTransform = _entities.at(currSelection).item->GetTransform();

			XMFLOAT4A transformationVector = { 0, 0, 0, 0 };

			if (input.GetKey(KeyCode::D) == KeyState::Held)
				transformationVector.x += time.deltaTime;
			else if (input.GetKey(KeyCode::A) == KeyState::Held)
				transformationVector.x -= time.deltaTime;

			if (input.GetKey(KeyCode::Space) == KeyState::Held)
				transformationVector.y += time.deltaTime;
			else if (input.GetKey(KeyCode::X) == KeyState::Held)
				transformationVector.y -= time.deltaTime;

			if (input.GetKey(KeyCode::W) == KeyState::Held)
				transformationVector.z += time.deltaTime;
			else if (input.GetKey(KeyCode::S) == KeyState::Held)
				transformationVector.z -= time.deltaTime;

			if (isRotating)
				entityTransform->Rotate(transformationVector);
			else if (isScaling)
				entityTransform->ScaleAbsolute(transformationVector);
			else
				entityTransform->Move(transformationVector);
		}
	}

	if (!_camera->UpdateBuffers(context))
	{
		ErrMsg("Failed to update camera buffers!");
		return false;
	}

	if (!_spotLights->UpdateBuffers(context))
	{
		ErrMsg("Failed to update spotlight buffers!");
		return false;
	}

	if (!_cubemap.UpdateBuffers(context))
	{
		ErrMsg("Failed to update cubemap buffers!");
		return false;
	}

	for (int i = 0; i < _entities.size(); i++)
	{
		const SceneEntity &ent = _entities.at(i);

		if (!ent.item->Update(context, time, input))
		{
			ErrMsg(std::format("Failed to update entity #{}!", i));
			return false;
		}
	}

	return true;
}

bool Scene::Render(Graphics *graphics, const Time &time, const Input &input)
{
	if (!_initialized)
		return false;

	static bool hasSetCamera = false;
	if (input.GetKey(KeyCode::V) == KeyState::Pressed)
		_currCamera = -2;

	if (!hasSetCamera)
	{
		_currCameraPtr = _camera;
		if (!graphics->SetSpotlightCollection(_spotLights))
		{
			ErrMsg("Failed to set spotlight collection!");
			return false;
		}

		/*if (!graphics->SetPointLightCollection(_pointLights))
		{
			ErrMsg("Failed to set point light collection!");
			return false;
		}*/

		hasSetCamera = true;
	}
	else if (input.GetKey(KeyCode::C) == KeyState::Pressed || 
			 input.GetKey(KeyCode::V) == KeyState::Pressed)
	{ // Change camera
		_currCamera++;

		if (_currCamera - 6 >= static_cast<int>(_spotLights->GetNrOfLights()))
			_currCamera = -1;

		if (_currCamera < 0)
		{
			_currCamera = -1;
			_currCameraPtr = _camera;
		}
		else if (_currCamera < 6)
			_currCameraPtr = _cubemap.GetCamera(_currCamera);
		else
			_currCameraPtr = _spotLights->GetLightCamera(_currCamera - 6);
	}

	if (!graphics->SetCamera(_currCameraPtr))
	{
		ErrMsg("Failed to set camera!");
		return false;
	}

	for (int i = 0; i < _entities.size(); i++)
	{
		const SceneEntity &ent = _entities.at(i);

		if (!ent.item->Render(graphics))
		{
			ErrMsg(std::format("Failed to render entity #{}!", i));
			return false;
		}
	}

	return true;
}