#include "Scene.h"

#include <cstdlib>

#include "ErrMsg.h"
#include "ImGui/imgui.h"


Scene::Scene()
{
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

	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 50, 0), XMFLOAT3(150, 500, 150));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	if (!_camera->Initialize(device, 
		{ 70.0f * (XM_PI / 180.0f), 16.0f / 9.0f, 0.05f, 50.0f }, 
		{ 0.0f, 1.5f, -1.0f, 0.0f }))
	{
		ErrMsg("Failed to initialize camera!");
		return false;
	}
	
	if (!_cubemap.Initialize(device, 1024, 0.1f, 50.0f, {0.0f, 0.0f, 0.0f, 0.0f}))
	{
		ErrMsg("Failed to initialize cubemap!");
		return false;
	}


	const SpotLightData spotLightInfo = {
		2048,
		std::vector<SpotLightData::PerLightInfo> {
			SpotLightData::PerLightInfo {
				{ 15.0f, 0.0f, 0.0f },		// color
				0.0f,						// rotationX
				0.0f,						// rotationY
				XM_PI * 0.4f,				// angle
				1.0f,						// falloff
				8.0f,						// specularity
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 2.5f, -3.25f }		// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 15.0f, 0.0f },		// color
				0.0f,						// rotationX
				0.3f,						// rotationY
				XM_PI * 0.4f,				// angle
				1.0f,						// falloff
				32.0f,						// specularity
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 3.5f, -3.25f }		// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 0.0f, 15.0f },		// color
				0.0f,						// rotationX
				-0.3f,						// rotationY
				XM_PI * 0.4f,				// angle
				1.0f,						// falloff
				128.0f,						// specularity
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
				{ 0.0f, 1.5f, -3.25f }		// initialPosition
			},

			SpotLightData::PerLightInfo {
				{ 20.0f, 20.0f, 20.0f },	// color
				0.0f,						// rotationX
				XM_PIDIV2,					// rotationY
				XM_PI * 0.5f,				// angle
				1.0f,						// falloff
				1024.0f,					// specularity
				0.05f,						// projectionNearZ
				30.0f,						// projectionFarZ
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


	// Create room
	{
		constexpr UINT
			meshID = 2,
			textureID = 2;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID))
		{
			ErrMsg("Failed to initialize room object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->ScaleRelative({ 15.0f, 15.0f, 15.0f, 0 });
	}

	// Create model
	{
		constexpr UINT
			meshID = 7,
			textureID = 7;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID))
		{
			ErrMsg("Failed to initialize model object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ 0.0f, 0.0f, 0.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->Rotate({ 0.0f, XM_PI, 0.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->ScaleRelative({ 0.3f, 0.3f, 0.3f, 0 });
	}

	// Create error
	{
		constexpr UINT
			meshID = 0,
			textureID = 1;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID))
		{
			ErrMsg("Failed to initialize error object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ -4.0f, 3.0f, 7.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->Rotate({ 0.0f, -XM_PIDIV2, 0.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->ScaleRelative({ 1.2f, 1.2f, 1.2f, 0 });
	}

	// Create emitter
	{
		Emitter *emitter = reinterpret_cast<Emitter *>(_sceneHolder.AddEntity(DirectX::BoundingBox({0,0,0}, {1,1,1}), EntityType::EMITTER));

		EmitterData emitterData = { };
		emitterData.particleCount = 512;
		emitterData.particleRate = 1;
		emitterData.lifetime = 1.0f;

		if (!emitter->Initialize(_device, emitterData, 10))
		{
			ErrMsg("Failed to initialize emitter!");
			return false;
		}

		reinterpret_cast<Entity *>(emitter)->GetTransform()->Move({ 0.0f, 4.0f, 0.0f, 0 });
	}

	// Create transparent
	{
		constexpr UINT
			meshID = 6,
			textureID = 8;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, true))
		{
			ErrMsg("Failed to initialize transparent object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ 1.5f, 1.0f, 4.0f, 0 });
	}
	
	_initialized = true;
	return true;
}


bool Scene::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	if (!_initialized)
		return false;


	/*_spotLights->GetLightCamera(0)->MoveRight(time.deltaTime * -1.5f);
	_spotLights->GetLightCamera(0)->LookX(time.deltaTime * 0.5f);

	_spotLights->GetLightCamera(1)->MoveRight(time.deltaTime * -1.5f * -1.4786f);
	_spotLights->GetLightCamera(1)->LookX(time.deltaTime * 0.5f * -1.4786f);

	_spotLights->GetLightCamera(2)->MoveRight(time.deltaTime * -1.5f * 1.84248f);
	_spotLights->GetLightCamera(2)->LookX(time.deltaTime * 0.5f * 1.84248f);*/
	

	if (input.IsInFocus()) // Handle user input while window is in focus
	{
		static UINT
			selectedMeshID = 0,
			selectedTextureID = 0;

		if (input.GetKey(KeyCode::Up) == KeyState::Pressed)
		{
			if (input.GetKey(KeyCode::M) == KeyState::Held)
				selectedMeshID = (selectedMeshID + 1) % _content->GetMeshCount();
			if (input.GetKey(KeyCode::T) == KeyState::Held)
				selectedTextureID = (selectedTextureID + 1) % _content->GetTextureCount();
		}

		if (input.GetKey(KeyCode::P) == KeyState::Pressed)
		{ // Create 10 random entities
			for (size_t i = 0; i < 25; i++)
			{
				const UINT
					meshID = rand() % _content->GetMeshCount(),
					textureID = rand() % _content->GetTextureCount();

				Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
				if (!obj->Initialize(_device, meshID, textureID, (textureID >= 8)))
				{
					ErrMsg(std::format("Failed to initialize entity #{}!", reinterpret_cast<Entity *>(obj)->GetID()));
					return false;
				}

				reinterpret_cast<Entity *>(obj)->GetTransform()->Move({
					static_cast<float>((rand() % 2000) - 1000) / 10.0f,
					static_cast<float>((rand() % 1000)) / 10.0f,
					static_cast<float>((rand() % 2000) - 1000) / 10.0f,
					0
				});

				reinterpret_cast<Entity *>(obj)->GetTransform()->Rotate({
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					0
				});
			}
		}
		else if (input.GetKey(KeyCode::O) == KeyState::Pressed)
		{ // Create one random entity in front of the camera
			Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(selectedMeshID)->GetBoundingBox(), EntityType::OBJECT));
			if (!obj->Initialize(_device, selectedMeshID, selectedTextureID, (selectedTextureID >= 8)))
			{
				ErrMsg(std::format("Failed to initialize entity #{}!", reinterpret_cast<Entity *>(obj)->GetID()));
				return false;
			}

			XMFLOAT4A camForward = _currCameraPtr->GetForward();
			*reinterpret_cast<XMVECTOR *>(&camForward) *= 3.0f;
			*reinterpret_cast<XMVECTOR *>(&camForward) += *reinterpret_cast<const XMVECTOR *>(&_currCameraPtr->GetPosition());

			reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition(camForward);
			reinterpret_cast<Entity *>(obj)->GetTransform()->SetAxes(
				_currCameraPtr->GetRight(),
				_currCameraPtr->GetUp(),
				_currCameraPtr->GetForward()
			);
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

		if (input.IsCursorLocked())
		{
			if (input.GetKey(KeyCode::Add) == KeyState::Pressed)
				currSelection++;
			if (input.GetKey(KeyCode::Subtract) == KeyState::Pressed)
				currSelection--;

			float currSpeed = 3.0f;
			if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
				currSpeed = 6.5f;
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
				if (mState.dx != 0) _currCameraPtr->LookX(static_cast<float>(mState.dx) / 360.0f);
				if (mState.dy != 0) _currCameraPtr->LookY(static_cast<float>(mState.dy) / 360.0f);
			}
			else
			{ // Move selected entity
				static bool relativeToCamera = false;
				if (input.GetKey(KeyCode::M) == KeyState::Pressed)
					relativeToCamera = !relativeToCamera;

				XMVECTOR right, up, forward;

				if (relativeToCamera)
				{
					right = *reinterpret_cast<const XMVECTOR *>(&_currCameraPtr->GetRight());
					up = *reinterpret_cast<const XMVECTOR *>(&_currCameraPtr->GetUp());
					forward = *reinterpret_cast<const XMVECTOR *>(&_currCameraPtr->GetForward());
				}
				else
				{
					right = XMVectorSet(1, 0, 0, 0);
					up = XMVectorSet(0, 1, 0, 0);
					forward = XMVectorSet(0, 0, 1, 0);
				}

				static bool isRotating = false;
				if (input.GetKey(KeyCode::R) == KeyState::Pressed)
					isRotating = !isRotating;
				static bool isScaling = false;
				if (input.GetKey(KeyCode::T) == KeyState::Pressed)
					isScaling = !isScaling;

				XMVECTOR transformationVector = XMVectorZero();
				bool doMove = false;

				if (input.GetKey(KeyCode::D) == KeyState::Held)
				{
					transformationVector += right * time.deltaTime * currSpeed;
					doMove = true;
				}
				else if (input.GetKey(KeyCode::A) == KeyState::Held)
				{
					transformationVector -= right * time.deltaTime * currSpeed;
					doMove = true;
				}

				if (input.GetKey(KeyCode::Space) == KeyState::Held)
				{
					transformationVector += up * time.deltaTime * currSpeed;
					doMove = true;
				}
				else if (input.GetKey(KeyCode::X) == KeyState::Held)
				{
					transformationVector -= up * time.deltaTime * currSpeed;
					doMove = true;
				}

				if (input.GetKey(KeyCode::W) == KeyState::Held)
				{
					transformationVector += forward * time.deltaTime * currSpeed;
					doMove = true;
				}
				else if (input.GetKey(KeyCode::S) == KeyState::Held)
				{
					transformationVector -= forward * time.deltaTime * currSpeed;
					doMove = true;
				}

				if (doMove)
				{
					Entity *ent = _sceneHolder.GetEntity(currSelection);
					Transform *entityTransform = ent->GetTransform();

					if (isRotating)
						entityTransform->Rotate(*reinterpret_cast<XMFLOAT4A *>(&transformationVector));
					else if (isScaling)
						entityTransform->ScaleAbsolute(*reinterpret_cast<XMFLOAT4A *>(&transformationVector));
					else
						entityTransform->Move(*reinterpret_cast<XMFLOAT4A *>(&transformationVector));


					if (!_sceneHolder.UpdateEntityPosition(ent))
					{
						ErrMsg("Failed to update entity position!");
						return false;
					}
				}
			}
		}
	}


	//if (!_spotLights->ScaleLightFrustumsToCamera(*_currCameraPtr))
	if (!_spotLights->ScaleLightFrustumsToCamera(*_camera))
	{
		ErrMsg("Failed to scale light frustums to camera!");
		return false;
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

	if (!_content->GetShader("CS_Particle")->BindShader(context))
	{
		ErrMsg(std::format("Failed to bind particle compute shader!"));
		return false;
	}

	const UINT entityCount = _sceneHolder.GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		if (!_sceneHolder.GetEntity(i)->Update(context, time, input))
		{
			ErrMsg(std::format("Failed to update entity #{}!", i));
			return false;
		}
	}

	if (!_sceneHolder.Update())
	{
		ErrMsg("Failed to update scene holder!");
		return false;
	}

	return true;
}

bool Scene::Render(Graphics *graphics, Time &time, const Input &input)
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
	else if (input.GetKey(KeyCode::C) == KeyState::Pressed || input.GetKey(KeyCode::V) == KeyState::Pressed)
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

	std::vector<Entity *> entitiesToRender;
	entitiesToRender.reserve(_currCameraPtr->GetCullCount());

	DirectX::BoundingFrustum viewFrustum;
	_currCameraPtr->StoreFrustum(viewFrustum);

	time.TakeSnapshot("FrustumCull");
	if (!_sceneHolder.FrustumCull(viewFrustum, entitiesToRender))
	{
		ErrMsg("Failed to perform frustum culling!");
		return false;
	}
	time.TakeSnapshot("FrustumCull");

	for (Entity *ent : entitiesToRender)
	{
		if (!ent->Render(_currCameraPtr))
		{
			ErrMsg("Failed to render entity!");
			return false;
		}
	}

	const int spotlightCount = static_cast<int>(_spotLights->GetNrOfLights());
	time.TakeSnapshot("FrustumCullSpotlights");

	if (_doMultiThread)
		#pragma omp parallel for num_threads(2)
		for (int i = 0; i < spotlightCount; i++)
		{
			if (!_spotLights->IsEnabled(i))
				continue; // Skip frustum culling if the spotlight is disabled

			CameraD3D11 *spotlightCamera = _spotLights->GetLightCamera(i);

			std::vector<Entity *> entitiesToCastShadows;
			entitiesToCastShadows.reserve(spotlightCamera->GetCullCount());

			DirectX::BoundingFrustum spotlightFrustum;
			spotlightCamera->StoreFrustum(spotlightFrustum);

			if (!viewFrustum.Intersects(spotlightFrustum))
			{ // Skip rendering if the frustums don't intersect
				_spotLights->SetEnabled(i, false);
				continue;
			}
			//_spotLights->SetEnabled(i, true);

			if (!_sceneHolder.FrustumCull(spotlightFrustum, entitiesToCastShadows))
			{
				ErrMsg(std::format("Failed to perform frustum culling for spotlight #{}!", i));
				continue;
			}

			for (Entity *ent : entitiesToCastShadows)
			{
				if (!ent->Render(spotlightCamera))
				{
					ErrMsg(std::format("Failed to render entity for spotlight #{}!", i));
					break;
				}
			}
		}
	else
		for (int i = 0; i < spotlightCount; i++)
		{
			CameraD3D11 *spotlightCamera = _spotLights->GetLightCamera(i);

			std::vector<Entity *> entitiesToCastShadows;
			entitiesToCastShadows.reserve(spotlightCamera->GetCullCount());

			DirectX::BoundingFrustum spotlightFrustum;
			spotlightCamera->StoreFrustum(spotlightFrustum);

			if (!viewFrustum.Intersects(spotlightFrustum))
			{ // Skip rendering if the frustums don't intersect
				_spotLights->SetEnabled(i, false);
				continue; 
			}
			_spotLights->SetEnabled(i, true);

			time.TakeSnapshot(std::format("FrustumCullSpotlight{}", i));
			if (!_sceneHolder.FrustumCull(spotlightFrustum, entitiesToCastShadows))
			{
				ErrMsg(std::format("Failed to perform frustum culling for spotlight #{}!", i));
				return false;
			}
			time.TakeSnapshot(std::format("FrustumCullSpotlight{}", i));

			for (Entity *ent : entitiesToCastShadows)
			{
				if (!ent->Render(spotlightCamera))
				{
					ErrMsg(std::format( "Failed to render entity for spotlight #{}!", i));
					return false;
				}
			}
		}

	time.TakeSnapshot("FrustumCullSpotlights");




	return true;
}

bool Scene::RenderUI()
{
	ImGui::Text(std::format("Objects in scene: {}", _sceneHolder.GetEntityCount()).c_str());

	const XMFLOAT4A camPos = _currCameraPtr->GetPosition();
	char camXCoord[32]{}, camYCoord[32]{}, camZCoord[32]{};
	snprintf(camXCoord, sizeof(camXCoord), "%.2f", camPos.x);
	snprintf(camYCoord, sizeof(camYCoord), "%.2f", camPos.y);
	snprintf(camZCoord, sizeof(camZCoord), "%.2f", camPos.z);
	ImGui::Text(std::format("Cam pos: ({}, {}, {})", camXCoord, camYCoord, camZCoord).c_str());

	if (ImGui::Button(_doMultiThread ? "Threading On" : "Threading Off"))
		_doMultiThread = !_doMultiThread;

	ImGui::Separator();

	char nearPlane[16]{}, farPlane[16]{};
	for (int i = 0; i < _spotLights->GetNrOfLights(); i++)
	{
		const ProjectionInfo projInfo = _spotLights->GetLightCamera(i)->GetCurrProjectionInfo();
		snprintf(nearPlane, sizeof(nearPlane), "%.2f", projInfo.nearZ);
		snprintf(farPlane, sizeof(farPlane), "%.1f", projInfo.farZ);
		ImGui::Text(std::format("({}:{}) Planes Spotlight #{}", nearPlane, farPlane, i).c_str());
	}

	return true;
}
