#include "Scene.h"

#include <cstdlib>

#include "ErrMsg.h"


Scene::Scene()
{
	_initialized = false;

	_camera = new CameraD3D11();
	_spotLights = new SpotLightCollectionD3D11();
	_pointLights = new PointLightCollectionD3D11();
}

Scene::~Scene()
{
	const size_t entityCount = _entities.size();
	for (size_t i = 0; i < entityCount; i++)
		delete _entities.at(i);

	delete _pointLights;
	delete _spotLights;
	delete _camera;
}

bool Scene::Initialize(ID3D11Device *device)
{
	if (_initialized)
		return false;

	_device = device;

	if (!_camera->Initialize(device, { 75.0f * (XM_PI / 180.0f), 16.0f / 9.0f, 0.1f, 50.0f}, {0.0f, 0.0f, -2.0f, 0.0f}))
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
		1024, // Resolution
		{ // Vector
			{ // Light
				{ 15.0f, 15.0f, 15.0f },	// color
				0.0f,						// rotationX
				0.0f,						// rotationY
				XM_PIDIV2,					// angle
				0.05f,						// projectionNearZ
				50.0f,						// projectionFarZ
				{ 0.0f, 0.0f, 0.0f }		// initialPosition
			},

			{ // Light
				{ 0.0f, 25.0f, 0.0f },	// color
				1.0f,						// rotationX
				4.0f,						// rotationY
				XM_PI/3.0f,					// angle
				0.05f,						// projectionNearZ
				50.0f,						// projectionFarZ
				{ -4.0f, 5.0f, -5.0f }	// initialPosition
			},

			{ // Light
				{ 0.0f, 0.0f, 20.0f },	// color
				4.0f,						// rotationX
				1.2f,						// rotationY
				XM_PI/2.5f,					// angle
				0.05f,						// projectionNearZ
				50.0f,						// projectionFarZ
				{ 15.0f, 15.0f, 0.0f }	// initialPosition
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
		_entities.push_back(new Entity(static_cast<UINT>(_entities.size())));
		Entity *ent = _entities.back();

		if (!ent->Initialize(_device, 0, 1, 0, 2, 2))
		{
			ErrMsg("Failed to initialize room entity!");
			return false;
		}

		ent->GetTransform()->ScaleRelative({ -15.0f, -15.0f, -15.0f, 0 });
	}

	// Create debug box
	{
		_entities.push_back(new Entity(static_cast<UINT>(_entities.size())));
		Entity *ent = _entities.back();

		if (!ent->Initialize(_device, 0, 1, 0, 2, 1))
		{
			ErrMsg("Failed to initialize box!");
			return false;
		}

		ent->GetTransform()->Move({ -1.0f, -0.5f, 5.0f, 0 });
		ent->GetTransform()->Rotate({ 0.26f, 1.9f, 3.91f, 0 });
		ent->GetTransform()->ScaleRelative({ 1.0f, 0.3f, 3.0f, 0 });
	}

	// Create debug error
	{
		_entities.push_back(new Entity(static_cast<UINT>(_entities.size())));
		Entity *ent = _entities.back();

		if (!ent->Initialize(_device, 0, 0, 0, 2, 0))
		{
			ErrMsg("Failed to initialize error!");
			return false;
		}

		ent->GetTransform()->Move({ 1.5f, -2.0f, 23.0f, 0 });
	}

	_initialized = true;
	return true;
}


bool Scene::Update(ID3D11DeviceContext *context, const Time &time, const Input &input)
{
	if (!_initialized)
		return false;

	if (input.IsCursorLocked()) // Handle user input
	{
		if (input.GetKey(KeyCode::P) == KeyState::Pressed)
		{ // Create 5 random entities
			for (size_t i = 0; i < 25; i++)
			{
				_entities.push_back(new Entity(static_cast<UINT>(_entities.size())));
				Entity *ent = _entities.back();

				if (!ent->Initialize(
					_device, 
					0, 
					rand() % 8, 
					0, 
					2, 
					rand() % 10))
				{
					ErrMsg(std::format("Failed to initialize entity #{}!", _entities.size() - 1));
					return false;
				}

				ent->GetTransform()->Move({
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					0
				});

				ent->GetTransform()->Rotate({
					static_cast<float>((rand() % 2000)) * (XM_PI / 1000.0f),
					static_cast<float>((rand() % 2000)) * (XM_PI / 1000.0f),
					static_cast<float>((rand() % 2000)) * (XM_PI / 1000.0f),
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
				_camera->MoveRight(time.deltaTime * 3.5f);
			else if (input.GetKey(KeyCode::A) == KeyState::Held)
				_camera->MoveRight(-time.deltaTime * 3.5f);

			if (input.GetKey(KeyCode::Space) == KeyState::Held)
				_camera->MoveUp(time.deltaTime * 3.5f);
			else if (input.GetKey(KeyCode::X) == KeyState::Held)
				_camera->MoveUp(-time.deltaTime * 3.5f);

			if (input.GetKey(KeyCode::W) == KeyState::Held)
				_camera->MoveForward(time.deltaTime * 3.5f);
			else if (input.GetKey(KeyCode::S) == KeyState::Held)
				_camera->MoveForward(-time.deltaTime * 3.5f);

			const MouseState mState = input.GetMouse();
			_camera->LookX(static_cast<float>(mState.dx) / 360.0f);
			_camera->LookY(static_cast<float>(mState.dy) / 360.0f);
		}
		else
		{ // Move selected entity
			static bool isRotating = false;
			if (input.GetKey(KeyCode::R) == KeyState::Pressed)
				isRotating = !isRotating;
			static bool isScaling = false;
			if (input.GetKey(KeyCode::T) == KeyState::Pressed)
				isScaling = !isScaling;

			Transform *entityTransform = _entities.at(currSelection)->GetTransform();

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
		Entity *ent = _entities.at(i);

		if (!ent->Update(context, time, input))
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

	static int currCamera = -1;
	if (input.GetKey(KeyCode::V) == KeyState::Pressed)
		currCamera = -2;

	if (!hasSetCamera)
	{
		if (!graphics->SetCamera(_camera))
		{
			ErrMsg("Failed to set camera!");
			return false;
		}

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
		currCamera++;

		if (currCamera - 6 >= static_cast<int>(_spotLights->GetNrOfLights()))
			currCamera = -1;

		if (currCamera < 0)
		{
			currCamera = -1;
			if (!graphics->SetCamera(_camera))
			{
				ErrMsg("Failed to set camera to main!");
				return false;
			}
		}
		else if (currCamera < 6)
		{
			if (!graphics->SetCamera(_cubemap.GetCamera(currCamera)))
			{
				ErrMsg(std::format("Failed to set camera to cubemap view #{}!", currCamera));
				return false;
			}
		}
		else
		{
			if (!graphics->SetCamera(_spotLights->GetLightCamera(currCamera - 6)))
			{
				ErrMsg(std::format("Failed to set camera to spotlight view #{}!", currCamera - 6));
				return false;
			}
		}
	}

	for (int i = 0; i < _entities.size(); i++)
	{
		Entity *ent = _entities.at(i);

		if (!ent->Render(graphics))
		{
			ErrMsg(std::format("Failed to render entity #{}!", i));
			return false;
		}
	}

	return true;
}