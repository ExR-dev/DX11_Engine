#include "Scene.h"

#include <cstdlib>

#include "ErrMsg.h"
#include "ImGui/imgui.h"


Scene::Scene()
{
	_camera = new CameraD3D11();
	_spotlights = new SpotLightCollectionD3D11();
	_pointlights = new PointLightCollectionD3D11();

	_currCameraPtr = _camera;
}

Scene::~Scene()
{
	delete _pointlights;
	delete _spotlights;
	delete _camera;
}

bool Scene::Initialize(ID3D11Device *device, Content *content)
{
	if (_initialized)
		return false;

	_device = device;
	_content = content;

	// Create scene content holder
	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 50, 0), XMFLOAT3(150, 150, 150));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	// Create camera
	if (!_camera->Initialize(device, 
		{ 70.0f * (XM_PI / 180.0f), 16.0f / 9.0f, 0.05f, 50.0f }, 
		{ 0.0f, 2.0f, -2.0f, 0.0f }))
	{
		ErrMsg("Failed to initialize camera!");
		return false;
	}

	// Create spotlights
	const SpotLightData spotlightInfo = {
		1024,
		std::vector<SpotLightData::PerLightInfo> {
			SpotLightData::PerLightInfo {
				{ 4.0f, 2.5f, 0.0f },		// initialPosition
				{ 15.0f, 0.0f, 0.0f },		// color
				-XM_PIDIV2,					// rotationX
				0.0f,						// rotationY
				XM_PI * 0.5f,				// angle
				1.0f,						// falloff
				32.0f,						// specularity
				0.1f,						// projectionNearZ
				35.0f						// projectionFarZ
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 6.5f, 0.0f },		// initialPosition
				{ 0.0f, 15.0f, 0.0f },		// color
				0.0f,						// rotationX
				XM_PIDIV2,					// rotationY
				XM_PI * 0.5f,				// angle
				1.0f,						// falloff
				32.0f,						// specularity
				0.1f,						// projectionNearZ
				35.0f						// projectionFarZ
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 2.5f, 4.0f },		// initialPosition
				{ 0.0f, 0.0f, 15.0f },		// color
				XM_PI,						// rotationX
				0.0f,						// rotationY
				XM_PI * 0.5f,				// angle
				1.0f,						// falloff
				32.0f,						// specularity
				0.1f,						// projectionNearZ
				35.0f						// projectionFarZ
			},
			
			SpotLightData::PerLightInfo {
				{ 0.0f, 20.0f, 0.0f },		// initialPosition
				{ 20.0f, 20.0f, 20.0f },	// color
				0.0f,						// rotationX
				XM_PIDIV2,					// rotationY
				XM_PI * 0.5f,				// angle
				0.75f,						// falloff
				512.0f,						// specularity
				0.1f,						// projectionNearZ
				25.0f						// projectionFarZ
			},
		}
	};

	if (!_spotlights->Initialize(device, spotlightInfo))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}


	// Create pointlights
	const PointLightData pointlightInfo = {
		512,
		std::vector<PointLightData::PerLightInfo> {
			PointLightData::PerLightInfo {
				{ 7.0f, 5.0f, -9.0f },			// initialPosition
				{ 15.0f, 15.0f, 15.0f },		// color
				3.0f,							// falloff
				32.0f,							// specularity
				0.1f,							// projectionNearZ
				35.0f							// projectionFarZ
			},

			PointLightData::PerLightInfo {
				{ -6.0f, 24.0f, -14.0f },		// initialPosition
				{ 0.0f, 20.0f, 15.0f },			// color
				4.0f,							// falloff
				32.0f,							// specularity
				0.1f,							// projectionNearZ
				20.0f							// projectionFarZ
			},
		}
	};

	if (!_pointlights->Initialize(device, pointlightInfo))
	{
		ErrMsg("Failed to initialize pointlight collection!");
		return false;
	}


	// Create cubemap
	if (!_cubemap.Initialize(device, 128, 0.1f, 25.0f, { 0.0f, 15.0f, 0.0f, 0.0f }))
	{
		ErrMsg("Failed to initialize cubemap!");
		return false;
	}


	// Create room
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Room"),
			textureID = content->GetTextureID("Tex_texture1"),
			normalID = CONTENT_LOAD_ERROR,
			specularID = CONTENT_LOAD_ERROR,
			reflectiveID = CONTENT_LOAD_ERROR,
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_LOAD_ERROR;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, normalID, specularID, heightID, ambientID, reflectiveID))
		{
			ErrMsg("Failed to initialize room object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->ScaleRelative({ 15.0f, 15.0f, 15.0f, 0 });
	}

	// Create model
	{
		const UINT
			meshID = content->GetMeshID("Mesh_CharacterSculptLow1"),
			textureID = content->GetTextureID("Tex_CharacterSculptLow0Texture1"),
			normalID = CONTENT_LOAD_ERROR,
			specularID = content->GetTextureMapID("TexMap_Gray_Specular"),
			reflectiveID = CONTENT_LOAD_ERROR,
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_LOAD_ERROR;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize model object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ 0.0f, 0.0f, 0.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->Rotate({ 0.0f, XM_PI, 0.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->ScaleRelative({ 0.3f, 0.3f, 0.3f, 0 });
	}

	// Create reflective sphere
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Sphere"),
			textureID = content->GetTextureID("Tex_White"),
			normalID = CONTENT_LOAD_ERROR,
			specularID = CONTENT_LOAD_ERROR,
			reflectiveID = content->GetTextureMapID("TexMap_White_Reflective"),
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_LOAD_ERROR;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize reflective sphere object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ 0.0f, 15.0f, 0.0f, 0.0f });
	}
	
	// Create PBR-sphere
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Sphere"),
			textureID = content->GetTextureID("Tex_Cobble"),
			normalID = content->GetTextureMapID("TexMap_Cobble_Normal"),
			specularID = content->GetTextureMapID("TexMap_Cobble_Specular"),
			reflectiveID = content->GetTextureMapID("TexMap_Cobble_Reflective"),
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = content->GetTextureMapID("TexMap_Cobble_Height");

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize PBR object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ 6.0f, 2.5f, -7.5f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f, 0 });
	}
	
	// Create specular sphere
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Sphere"),
			textureID = content->GetTextureID("Tex_White"),
			normalID = CONTENT_LOAD_ERROR,
			specularID = content->GetTextureMapID("TexMap_White_Specular"),
			reflectiveID = CONTENT_LOAD_ERROR,
			ambientID = CONTENT_LOAD_ERROR,
			heightID = CONTENT_LOAD_ERROR;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize specular object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ 6.0f, 6.0f, 7.5f, 0 });
	}

	// Create error
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Error"),
			textureID = content->GetTextureID("Tex_Fallback"),
			normalID = CONTENT_LOAD_ERROR,
			specularID = CONTENT_LOAD_ERROR,
			reflectiveID = CONTENT_LOAD_ERROR,
			ambientID = content->GetTextureID("Tex_Fallback"),
			heightID = CONTENT_LOAD_ERROR;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize error object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ -4.0f, 3.0f, 7.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->Rotate({ 0.0f, -XM_PIDIV2, 0.0f, 0 });
		reinterpret_cast<Entity *>(obj)->GetTransform()->ScaleRelative({ 1.2f, 1.2f, 1.2f, 0 });
	}

	// Create transparent
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Fallback"),
			textureID = content->GetTextureID("Tex_Transparent"),
			normalID = content->GetTextureMapID("TexMap_Bricks_Normal"),
			specularID = content->GetTextureMapID("TexMap_Bricks_Specular"),
			reflectiveID = CONTENT_LOAD_ERROR,
			ambientID = CONTENT_LOAD_ERROR,
			heightID = CONTENT_LOAD_ERROR;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID, true))
		{
			ErrMsg("Failed to initialize transparent object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->Move({ 2.0f, 1.5f, 4.0f, 0 });
	}

	// Create emitter
	{
		Emitter *emitter = reinterpret_cast<Emitter *>(_sceneHolder.AddEntity(DirectX::BoundingBox({0,0,0}, {15,15,15}), EntityType::EMITTER));

		EmitterData emitterData = { };
		emitterData.particleCount = 2048;
		emitterData.particleRate = 1;
		emitterData.lifetime = 5.0f;

		if (!emitter->Initialize(_device, emitterData, content->GetTextureID("Tex_Particle")))
		{
			ErrMsg("Failed to initialize emitter!");
			return false;
		}

		reinterpret_cast<Entity *>(emitter)->GetTransform()->Move({ 0.0f, 15.0f, 0.0f, 0 });
	}
	
	_initialized = true;
	return true;
}


bool Scene::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	if (!_initialized)
		return false;

	static bool rotateLights = false;
	if (input.GetKey(KeyCode::G) == KeyState::Pressed)
		rotateLights = !rotateLights;

	if (rotateLights)
	{
		_spotlights->GetLightCamera(0)->LookY(time.deltaTime * 0.5f);
		_spotlights->GetLightCamera(0)->MoveUp(time.deltaTime * 2.0f);

		_spotlights->GetLightCamera(1)->LookY(time.deltaTime * 0.5f);
		_spotlights->GetLightCamera(1)->MoveUp(time.deltaTime * 2.0f);

		_spotlights->GetLightCamera(2)->LookX(time.deltaTime * 0.5f);
		_spotlights->GetLightCamera(2)->MoveRight(time.deltaTime * -2.0f);
	}
	

	if (input.IsInFocus()) // Handle user input while window is in focus
	{
		static bool useMainCamera = true;
		if (input.GetKey(KeyCode::Z) == KeyState::Pressed)
			useMainCamera = !useMainCamera;
		else if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			useMainCamera = true;

		CameraD3D11 *basisCamera = _currCameraPtr;
		if (useMainCamera) basisCamera = _camera;

		static UINT
			selectedMeshID = 0,
			selectedTextureID = 0;

		if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
		{
			selectedMeshID = 0;
			selectedTextureID = 0;
		}

		if (input.GetKey(KeyCode::Up) == KeyState::Pressed)
		{
			if (input.GetKey(KeyCode::M) == KeyState::Held)
				selectedMeshID = (selectedMeshID + 1) % _content->GetMeshCount();
			if (input.GetKey(KeyCode::T) == KeyState::Held)
				selectedTextureID = (selectedTextureID + 1) % _content->GetTextureCount();
		}

		const UINT transparentStart = _content->GetTextureID("Tex_Transparent");
		if (input.GetKey(KeyCode::P) == KeyState::Pressed)
		{ // Create 25 random entities within the scene bounds
			const DirectX::BoundingBox sceneBounds = _sceneHolder.GetBounds();
			const DirectX::XMFLOAT3
				sceneCenter = sceneBounds.Center,
				sceneExtents = sceneBounds.Extents;

			for (size_t i = 0; i < 25; i++)
			{
				const UINT
					meshID = rand() % _content->GetMeshCount(),
					textureID = rand() % _content->GetTextureCount();

				Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingBox(), EntityType::OBJECT));
				if (!obj->Initialize(_device, 
					meshID, textureID, 
					CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, 
					CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, 
					(textureID >= transparentStart)))
				{
					ErrMsg(std::format("Failed to initialize entity #{}!", reinterpret_cast<Entity *>(obj)->GetID()));
					return false;
				}

				reinterpret_cast<Entity *>(obj)->GetTransform()->Move({
					sceneCenter.x + sceneExtents.x * static_cast<float>((rand() % 2000) - 1000) / 1000.0f,
					sceneCenter.y + sceneExtents.y * static_cast<float>((rand() % 2000) - 1000) / 1000.0f,
					sceneCenter.z + sceneExtents.z * static_cast<float>((rand() % 2000) - 1000) / 1000.0f,
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
		{ // Create one custom entity in front of the camera
			Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(selectedMeshID)->GetBoundingBox(), EntityType::OBJECT));
			if (!obj->Initialize(_device, 
				selectedMeshID, selectedTextureID, 
				CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, 
				CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, 
				(selectedTextureID >= transparentStart)))
			{
				ErrMsg(std::format("Failed to initialize entity #{}!", reinterpret_cast<Entity *>(obj)->GetID()));
				return false;
			}

			XMFLOAT4A camForward = basisCamera->GetForward();
			*reinterpret_cast<XMVECTOR *>(&camForward) *= 3.0f;
			*reinterpret_cast<XMVECTOR *>(&camForward) += *reinterpret_cast<const XMVECTOR *>(&basisCamera->GetPosition());

			reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition(camForward);
			reinterpret_cast<Entity *>(obj)->GetTransform()->SetAxes(
				basisCamera->GetRight(),
				basisCamera->GetUp(),
				basisCamera->GetForward()
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

		if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			currSelection = -1;

		if (input.IsCursorLocked())
		{
			if (input.GetKey(KeyCode::Add) == KeyState::Pressed)
				currSelection++;
			if (input.GetKey(KeyCode::Subtract) == KeyState::Pressed)
				currSelection--;

			if (currSelection < -1)
				currSelection = -1;
			else if (currSelection >= (int)_sceneHolder.GetEntityCount())
				currSelection = (int)_sceneHolder.GetEntityCount() - 1;

			float currSpeed = 3.0f;
			if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
				currSpeed = 6.5f;
			if (input.GetKey(KeyCode::LeftControl) == KeyState::Held)
				currSpeed = 0.5f;

			if (currSelection == -1)
			{ // Move camera
				if (input.GetKey(KeyCode::D) == KeyState::Held)
					basisCamera->MoveRight(time.deltaTime * currSpeed);
				else if (input.GetKey(KeyCode::A) == KeyState::Held)
					basisCamera->MoveRight(-time.deltaTime * currSpeed);

				if (input.GetKey(KeyCode::Space) == KeyState::Held)
					basisCamera->MoveUp(time.deltaTime * currSpeed);
				else if (input.GetKey(KeyCode::X) == KeyState::Held)
					basisCamera->MoveUp(-time.deltaTime * currSpeed);

				if (input.GetKey(KeyCode::W) == KeyState::Held)
					basisCamera->MoveForward(time.deltaTime * currSpeed);
				else if (input.GetKey(KeyCode::S) == KeyState::Held)
					basisCamera->MoveForward(-time.deltaTime * currSpeed);

				const MouseState mState = input.GetMouse();
				if (mState.dx != 0) basisCamera->LookX(static_cast<float>(mState.dx) / 360.0f);
				if (mState.dy != 0) basisCamera->LookY(static_cast<float>(mState.dy) / 360.0f);
			}
			else
			{ // Move selected entity
				static bool relativeToCamera = false;
				if (input.GetKey(KeyCode::M) == KeyState::Pressed)
					relativeToCamera = !relativeToCamera;

				XMVECTOR right, up, forward;

				if (relativeToCamera)
				{
					right = *reinterpret_cast<const XMVECTOR *>(&basisCamera->GetRight());
					up = *reinterpret_cast<const XMVECTOR *>(&basisCamera->GetUp());
					forward = *reinterpret_cast<const XMVECTOR *>(&basisCamera->GetForward());
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

	if (!_spotlights->UpdateBuffers(context))
	{
		ErrMsg("Failed to update spotlight buffers!");
		return false;
	}

	if (!_pointlights->UpdateBuffers(context))
	{
		ErrMsg("Failed to update pointlight buffers!");
		return false;
	}

	if (_updateCubemap)
		if (!_cubemap.Update(context, time))
		{
			ErrMsg("Failed to update cubemap!");
			return false;
		}

	static UINT particleShaderID = _content->GetShaderID("CS_Particle");
	if (!_content->GetShader(particleShaderID)->BindShader(context))
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

	_updateCubemap = graphics->GetUpdateCubemap();

	static bool hasSetCamera = false;
	if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
		_currCamera = -2;

	if (!hasSetCamera)
	{
		_currCameraPtr = _camera;
		if (!graphics->SetSpotlightCollection(_spotlights))
		{
			ErrMsg("Failed to set spotlight collection!");
			return false;
		}

		if (!graphics->SetPointlightCollection(_pointlights))
		{
			ErrMsg("Failed to set pointlight collection!");
			return false;
		}

		hasSetCamera = true;
	}
	else if (input.GetKey(KeyCode::C) == KeyState::Pressed || _currCamera == -2)
	{ // Change camera
		_currCamera++;

		if (_currCamera - 6 >= static_cast<int>(_spotlights->GetNrOfLights()))
			_currCamera = -1;

		if (_currCamera < 0)
		{
			_currCamera = -1;
			_currCameraPtr = _camera;
		}
		else if (_currCamera < 6)
			_currCameraPtr = _cubemap.GetCamera(_currCamera);
		else
			_currCameraPtr = _spotlights->GetLightCamera(_currCamera - 6);
	}

	if (!graphics->SetCameras(_camera, _currCameraPtr))
	{
		ErrMsg("Failed to set camera!");
		return false;
	}

	std::vector<Entity *> entitiesToRender;
	entitiesToRender.reserve(_camera->GetCullCount());

	DirectX::BoundingFrustum viewFrustum;
	if (!_camera->StoreBounds(viewFrustum))
	{
		ErrMsg("Failed to store camera frustum!");
		return false;
	}

	time.TakeSnapshot("FrustumCull");
	if (!_sceneHolder.FrustumCull(viewFrustum, entitiesToRender))
	{
		ErrMsg("Failed to perform frustum culling!");
		return false;
	}

	for (Entity *ent : entitiesToRender)
	{
		if (!ent->Render(_camera))
		{
			ErrMsg("Failed to render entity!");
			return false;
		}
	}
	time.TakeSnapshot("FrustumCull");

	const int spotlightCount = static_cast<int>(_spotlights->GetNrOfLights());
	time.TakeSnapshot("FrustumCullSpotlights");
	if (_doMultiThread)
		#pragma omp parallel for num_threads(2)
		for (int i = 0; i < spotlightCount; i++)
		{
			CameraD3D11 *spotlightCamera = _spotlights->GetLightCamera(i);

			std::vector<Entity *> entitiesToCastShadows;
			entitiesToCastShadows.reserve(spotlightCamera->GetCullCount());

			DirectX::BoundingFrustum spotlightFrustum;
			if (!spotlightCamera->StoreBounds(spotlightFrustum))
			{
				ErrMsg("Failed to store spotlight camera frustum!");
				continue;
			}

			if (!viewFrustum.Intersects(spotlightFrustum) && !_cubemap.GetUpdate())
			{ // Skip rendering if the frustums don't intersect
				_spotlights->SetEnabled(i, false);
				continue;
			}
			_spotlights->SetEnabled(i, true);

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
			CameraD3D11 *spotlightCamera = _spotlights->GetLightCamera(i);

			std::vector<Entity *> entitiesToCastShadows;
			entitiesToCastShadows.reserve(spotlightCamera->GetCullCount());

			DirectX::BoundingFrustum spotlightFrustum;
			if (!spotlightCamera->StoreBounds(spotlightFrustum))
			{
				ErrMsg("Failed to store spotlight camera frustum!");
				return false;
			}

			if (!viewFrustum.Intersects(spotlightFrustum) && !_cubemap.GetUpdate())
			{ // Skip rendering if the frustums don't intersect
				_spotlights->SetEnabled(i, false);
				continue; 
			}
			_spotlights->SetEnabled(i, true);

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

	const int pointlightCount = static_cast<int>(_pointlights->GetNrOfLights());
	time.TakeSnapshot("FrustumCullPointlights");
	if (_doMultiThread)
		#pragma omp parallel for num_threads(2)
		for (int i = 0; i < pointlightCount; i++)
			for (int j = 0; j < 6; j++)
			{
				CameraD3D11 *pointlightCamera = _pointlights->GetLightCamera(i, j);

				std::vector<Entity *> entitiesToCastShadows;
				entitiesToCastShadows.reserve(pointlightCamera->GetCullCount());

				DirectX::BoundingFrustum pointlightFrustum;
				if (!pointlightCamera->StoreBounds(pointlightFrustum))
				{
					ErrMsg("Failed to store pointlight camera frustum!");
					continue;
				}

				if (!viewFrustum.Intersects(pointlightFrustum) && !_cubemap.GetUpdate())
				{ // Skip rendering if the frustums don't intersect
					_pointlights->SetEnabled(i, j, false);
					continue;
				}
				_pointlights->SetEnabled(i, j, true);

				if (!_sceneHolder.FrustumCull(pointlightFrustum, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform frustum culling for pointlight #{} camera #{}!", i, j));
					continue;
				}

				for (Entity *ent : entitiesToCastShadows)
				{
					if (!ent->Render(pointlightCamera))
					{
						ErrMsg(std::format("Failed to render entity for pointlight #{} camera #{}!", i, j));
						break;
					}
				}
			}
	else
		for (int i = 0; i < pointlightCount; i++)
		{
			time.TakeSnapshot(std::format("FrustumCullPointlight{}", i));
			for (int j = 0; j < 6; j++)
			{
				CameraD3D11 *pointlightCamera = _pointlights->GetLightCamera(i, j);

				std::vector<Entity *> entitiesToCastShadows;
				entitiesToCastShadows.reserve(pointlightCamera->GetCullCount());

				DirectX::BoundingFrustum pointlightFrustum;
				if (!pointlightCamera->StoreBounds(pointlightFrustum))
				{
					ErrMsg("Failed to store pointlight camera frustum!");
					return false;
				}

				if (!viewFrustum.Intersects(pointlightFrustum) && !_cubemap.GetUpdate())
				{ // Skip rendering if the frustums don't intersect
					_pointlights->SetEnabled(i, j, false);
					continue;
				}
				_pointlights->SetEnabled(i, j, true);

				if (!_sceneHolder.FrustumCull(pointlightFrustum, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform frustum culling for pointlight #{} camera #{}!", i, j));
					return false;
				}

				for (Entity *ent : entitiesToCastShadows)
				{
					if (!ent->Render(pointlightCamera))
					{
						ErrMsg(std::format("Failed to render entity for pointlight #{} camera #{}!", i, j));
						return false;
					}
				}
			}
			time.TakeSnapshot(std::format("FrustumCullPointlight{}", i));
		}
	time.TakeSnapshot("FrustumCullPointlights");


	time.TakeSnapshot("FrustumCullCubemap");
	if (_updateCubemap && _cubemap.GetUpdate())
	{
		if (_doMultiThread)
			#pragma omp parallel for num_threads(2)
			for (int i = 0; i < 6; i++)
			{
				CameraD3D11 *cubemapCamera = _cubemap.GetCamera(i);

				std::vector<Entity *> entitiesToReflect;
				entitiesToReflect.reserve(cubemapCamera->GetCullCount());

				DirectX::BoundingFrustum cubemapViewFrustum;
				entitiesToReflect.clear();
				entitiesToReflect.reserve(cubemapCamera->GetCullCount());

				if (!cubemapCamera->StoreBounds(cubemapViewFrustum))
				{
					ErrMsg("Failed to store cubemap camera frustum!");
					continue;
				}

				if (!_sceneHolder.FrustumCull(cubemapViewFrustum, entitiesToReflect))
				{
					ErrMsg(std::format("Failed to perform frustum culling for cubemap camera #{}!", i));
					continue;
				}

				for (Entity *ent : entitiesToReflect)
				{
					if (!ent->Render(cubemapCamera))
					{
						ErrMsg(std::format("Failed to render entity for cubemap camera #{}!", i));
						continue;
					}
				}
			}
		else
			for (int i = 0; i < 6; i++)
			{
				CameraD3D11 *cubemapCamera = _cubemap.GetCamera(i);

				entitiesToRender.clear();
				entitiesToRender.reserve(cubemapCamera->GetCullCount());

				if (!cubemapCamera->StoreBounds(viewFrustum))
				{
					ErrMsg("Failed to store cubemap camera frustum!");
					return false;
				}

				if (!_sceneHolder.FrustumCull(viewFrustum, entitiesToRender))
				{
					ErrMsg(std::format("Failed to perform frustum culling for cubemap camera #{}!", i));
					return false;
				}

				for (Entity *ent : entitiesToRender)
				{
					if (!ent->Render(cubemapCamera))
					{
						ErrMsg(std::format("Failed to render entity for cubemap camera #{}!", i));
						return false;
					}
				}
			}
	}
	time.TakeSnapshot("FrustumCullCubemap");

	if (!graphics->SetCubemap(&_cubemap))
	{
		ErrMsg("Failed to set cubemap!");
		return false;
	}

	return true;
}

bool Scene::RenderUI()
{
	ImGui::Text(std::format("Objects in scene: {}", _sceneHolder.GetEntityCount()).c_str());

	static char fovText[32] = "90.0";
	if (ImGui::InputText("FOV", fovText, 32))
	{
		float newFOV = static_cast<float>(atof(fovText));
		if (newFOV > 0.0f && newFOV < 180.0f)
			_camera->SetFOV(newFOV * (XM_PI / 180.0f));
	}

	XMFLOAT4A camPos = _camera->GetPosition();
	char camXCoord[32]{}, camYCoord[32]{}, camZCoord[32]{};
	snprintf(camXCoord, sizeof(camXCoord), "%.2f", camPos.x);
	snprintf(camYCoord, sizeof(camYCoord), "%.2f", camPos.y);
	snprintf(camZCoord, sizeof(camZCoord), "%.2f", camPos.z);
	ImGui::Text(std::format("Main Cam pos: ({}, {}, {})", camXCoord, camYCoord, camZCoord).c_str());

	if (_currCameraPtr != _camera && _currCameraPtr != nullptr)
	{
		camPos = _currCameraPtr->GetPosition();
		snprintf(camXCoord, sizeof(camXCoord), "%.2f", camPos.x);
		snprintf(camYCoord, sizeof(camYCoord), "%.2f", camPos.y);
		snprintf(camZCoord, sizeof(camZCoord), "%.2f", camPos.z);
		ImGui::Text(std::format("Virt Cam pos: ({}, {}, {})", camXCoord, camYCoord, camZCoord).c_str());
	}

	if (ImGui::Button(_doMultiThread ? "Threading On" : "Threading Off"))
		_doMultiThread = !_doMultiThread;

	ImGui::Separator();

	char nearPlane[16]{}, farPlane[16]{};
	for (UINT i = 0; i < _spotlights->GetNrOfLights(); i++)
	{
		const ProjectionInfo projInfo = _spotlights->GetLightCamera(i)->GetCurrProjectionInfo();
		snprintf(nearPlane, sizeof(nearPlane), "%.2f", projInfo.nearZ);
		snprintf(farPlane, sizeof(farPlane), "%.1f", projInfo.farZ);
		ImGui::Text(std::format("({}:{}) Planes Spotlight #{}", nearPlane, farPlane, i).c_str());
	}

	return true;
}
