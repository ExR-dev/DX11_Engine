#include "Scene.h"

#include <cstdlib>

#include "ErrMsg.h"
#include "ImGui/imgui.h"


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
	delete _pointLights;
	delete _spotLights;
	delete _camera;
}

bool Scene::Initialize(ID3D11Device *device, Content *content)
{
	if (_initialized)
		return false;

	_device = device;
	_content = content;

	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(32, 32, 32));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	if (!_camera->Initialize(device, 
		{ 70.0f * (XM_PI / 180.0f), 16.0f / 9.0f, 0.05f, 25.0f }, 
		{ 0.0f, 1.5f, -1.0f, 0.0f }))
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
				{ 7.0f, 0.0f, 0.0f },		// color
				0.0f,						// rotationX
				0.0f,						// rotationY
				XM_PI * 0.4f,				// angle
				15.0f,						// falloff
				8.0f,						// specularity
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 2.5f, -3.25f }	// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 7.0f, 0.0f },		// color
				0.0f,						// rotationX
				0.3f,						// rotationY
				XM_PI * 0.4f,				// angle
				10.0f,						// falloff
				32.0f,						// specularity
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 3.5f, -3.25f }	// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 0.0f, 7.0f },		// color
				0.0f,						// rotationX
				-0.3f,						// rotationY
				XM_PI * 0.4f,				// angle
				5.0f,						// falloff
				128.0f,						// specularity
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 1.5f, -3.25f }	// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 20.0f, 20.0f, 20.0f },	// color
				0.0f,						// rotationX
				XM_PIDIV2,					// rotationY
				XM_PI * 0.5f,				// angle
				10.5f,						// falloff
				1024.0f,					// specularity
				0.1f,						// projectionNearZ
				25.0f,						// projectionFarZ
				{ 0.0f, 20.0f, 0.0f }		// initialPosition
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
		constexpr UINT
			inputLayoutID = 0,
			meshID = 2,
			vShaderID = 0,
			pShaderID = 2,
			textureID = 2;

		Entity *ent = _sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox());
		if (!ent->Initialize(_device, inputLayoutID, meshID, vShaderID, pShaderID, textureID))
		{
			ErrMsg("Failed to initialize room entity!");
			return false;
		}

		ent->GetTransform()->ScaleRelative({ 15.0f, 15.0f, 15.0f, 0 });

		if (!_sceneHolder.UpdateEntityPosition(ent))
		{
			ErrMsg("Failed to update room position!");
			return false;
		}
	}

	// Create model
	{
		constexpr UINT
			inputLayoutID = 0,
			meshID = 6,
			vShaderID = 0,
			pShaderID = 2,
			textureID = 7;

		Entity *ent = _sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox());
		if (!ent->Initialize(_device, inputLayoutID, meshID, vShaderID, pShaderID, textureID))
		{
			ErrMsg("Failed to initialize model!");
			return false;
		}

		ent->GetTransform()->Move({ 0.0f, 0.0f, 0.0f, 0 });
		ent->GetTransform()->Rotate({ 0.0f, XM_PI, 0.0f, 0 });
		ent->GetTransform()->ScaleRelative({ 0.3f, 0.3f, 0.3f, 0 });

		if (!_sceneHolder.UpdateEntityPosition(ent))
		{
			ErrMsg("Failed to update model position!");
			return false;
		}
	}

	// Create error
	{
		constexpr UINT
			inputLayoutID = 0,
			meshID = 0,
			vShaderID = 0,
			pShaderID = 2,
			textureID = 1;

		Entity *ent = _sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox());
		if (!ent->Initialize(_device, inputLayoutID, meshID, vShaderID, pShaderID, textureID))
		{
			ErrMsg("Failed to initialize error!");
			return false;
		}

		ent->GetTransform()->Move({ -4.0f, 3.0f, 7.0f, 0 });
		ent->GetTransform()->Rotate({ 0.0f, -XM_PIDIV2, 0.0f, 0 });
		ent->GetTransform()->ScaleRelative({ 1.2f, 1.2f, 1.2f, 0 });

		if (!_sceneHolder.UpdateEntityPosition(ent))
		{
			ErrMsg("Failed to update error position!");
			return false;
		}
	}
	
	_initialized = true;
	return true;
}


bool Scene::Update(ID3D11DeviceContext *context, const Time &time, const Input &input)
{
	if (!_initialized)
		return false;

	
	_spotLights->GetLightCamera(0)->MoveRight(time.deltaTime * -1.5f);
	_spotLights->GetLightCamera(0)->LookX(time.deltaTime * 0.5f);

	_spotLights->GetLightCamera(1)->MoveRight(time.deltaTime * -1.5f * -1.4786f);
	_spotLights->GetLightCamera(1)->LookX(time.deltaTime * 0.5f * -1.4786f);

	_spotLights->GetLightCamera(2)->MoveRight(time.deltaTime * -1.5f * 1.84248f);
	_spotLights->GetLightCamera(2)->LookX(time.deltaTime * 0.5f * 1.84248f);
	

	if (input.IsCursorLocked()) // Handle user input
	{
		if (input.GetKey(KeyCode::P) == KeyState::Pressed)
		{ // Create 10 random entities
			for (size_t i = 0; i < 10; i++)
			{
				const UINT
					inputLayoutID = 0,
					meshID = rand() % _content->GetMeshCount(),
					vShaderID = 0,
					pShaderID = 2,
					textureID = rand() % _content->GetTextureCount();

				Entity *ent = _sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox());
				if (!ent->Initialize(_device, inputLayoutID, meshID, vShaderID, pShaderID, textureID))
				{
					ErrMsg(std::format("Failed to initialize entity #{}!", ent->GetID()));
					return false;
				}

				ent->GetTransform()->Move({
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					static_cast<float>((rand() % 2000) - 1000) / 60.0f,
					0
				});

				ent->GetTransform()->Rotate({
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					0
				});

				if (!_sceneHolder.UpdateEntityPosition(ent))
				{
					ErrMsg("Failed to update entity position!");
					return false;
				}
			}
		}

		static int currSelection = -1;
		const UINT entityCount = _sceneHolder.GetEntityCount();
		for (UCHAR i = 0; i < entityCount; i++)
		{
			if (i > 10)
				break;

			if (input.GetKey(static_cast<KeyCode>(static_cast<UCHAR>(KeyCode::D1) + i)) == KeyState::Pressed)
			{
				currSelection = (currSelection == i) ? -1 : i;
				break;
			}
		}

		if (input.GetKey(KeyCode::Add) == KeyState::Pressed)
			currSelection++;
		if (input.GetKey(KeyCode::Subtract) == KeyState::Pressed)
			currSelection--;

		float currSpeed = 2.5f;
		if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
			currSpeed = 4.5f;
		if (input.GetKey(KeyCode::LeftControl) == KeyState::Held)
			currSpeed = 0.5f;

		if (currSelection == -1)
		{ // Move camera
			if (input.GetKey(KeyCode::D) == KeyState::Held)
				_currCameraPtr->MoveRight(time.deltaTime * currSpeed);
			else if (input.GetKey(KeyCode::A) == KeyState::Held)
				_currCameraPtr->MoveRight(-time.deltaTime * currSpeed);

			if (input.GetKey(KeyCode::Space) == KeyState::Held)
				_currCameraPtr->MoveUp(time.deltaTime * currSpeed);
			else if (input.GetKey(KeyCode::X) == KeyState::Held)
				_currCameraPtr->MoveUp(-time.deltaTime * currSpeed);

			if (input.GetKey(KeyCode::W) == KeyState::Held)
				_currCameraPtr->MoveForward(time.deltaTime * currSpeed);
			else if (input.GetKey(KeyCode::S) == KeyState::Held)
				_currCameraPtr->MoveForward(-time.deltaTime * currSpeed);

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

			XMFLOAT4A transformationVector = { 0, 0, 0, 0 };

			if (input.GetKey(KeyCode::D) == KeyState::Held)
				transformationVector.x += time.deltaTime * currSpeed;
			else if (input.GetKey(KeyCode::A) == KeyState::Held)
				transformationVector.x -= time.deltaTime * currSpeed;

			if (input.GetKey(KeyCode::Space) == KeyState::Held)
				transformationVector.y += time.deltaTime * currSpeed;
			else if (input.GetKey(KeyCode::X) == KeyState::Held)
				transformationVector.y -= time.deltaTime * currSpeed;

			if (input.GetKey(KeyCode::W) == KeyState::Held)
				transformationVector.z += time.deltaTime * currSpeed;
			else if (input.GetKey(KeyCode::S) == KeyState::Held)
				transformationVector.z -= time.deltaTime * currSpeed;

			Entity *ent = _sceneHolder.GetEntity(currSelection);
			Transform *entityTransform = ent->GetTransform();

			if (isRotating)
				entityTransform->Rotate(transformationVector);
			else if (isScaling)
				entityTransform->ScaleAbsolute(transformationVector);
			else
				entityTransform->Move(transformationVector);

			if (!_sceneHolder.UpdateEntityPosition(ent))
			{
				ErrMsg("Failed to update entity position!");
				return false;
			}
		}
	}


	//if (!_spotLights->ScaleLightFrustumsToCamera(*_currCameraPtr))
	/*if (!_spotLights->ScaleLightFrustumsToCamera(*_camera))
	{
		ErrMsg("Failed to scale light frustums to camera!");
		return false;
	}*/


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

	const UINT entityCount = _sceneHolder.GetEntityCount();
	for (int i = 0; i < entityCount; i++)
	{
		if (!_sceneHolder.GetEntity(i)->Update(context, time, input))
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

	static std::set<Entity *> entitiesToRender;
	entitiesToRender.clear();

	DirectX::BoundingFrustum viewFrustum;
	_currCameraPtr->StoreFrustum(viewFrustum);

	if (!_sceneHolder.FrustumCull(viewFrustum, entitiesToRender))
	{
		ErrMsg("Failed to perform frustum culling!");
		return false;
	}

	for (Entity *ent : entitiesToRender)
	{
		if (!ent->Render(graphics))
		{
			ErrMsg("Failed to render entity!");
			return false;
		}
	}

	return true;
}

bool Scene::RenderUI() const
{
	if (_sceneHolder.GetEntityCount() > 150)
	{
		int i = 0;
		printf("");
	}

	ImGui::Text(std::format("Objects in scene: {}", _sceneHolder.GetEntityCount()).c_str());

	const XMFLOAT4A camPos = _currCameraPtr->GetPosition();
	char camXCoord[32]{}, camYCoord[32]{}, camZCoord[32]{};
	snprintf(camXCoord, sizeof(camXCoord), "%.2f", camPos.x);
	snprintf(camYCoord, sizeof(camYCoord), "%.2f", camPos.y);
	snprintf(camZCoord, sizeof(camZCoord), "%.2f", camPos.z);

	ImGui::Text(std::format("Cam pos: ({}, {}, {})", camXCoord, camYCoord, camZCoord).c_str());

	return true;
}
