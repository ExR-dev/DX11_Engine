#include "Scene.h"
#include "ErrMsg.h"
#include "ImGui/imgui.h"
#include <algorithm>
#include <cstdlib>
#include <memory>

using namespace DirectX;

Scene::Scene()
{
	_camera = std::unique_ptr<CameraD3D11>(new CameraD3D11());
	_secondaryCamera = std::unique_ptr<CameraD3D11>(new CameraD3D11());
	_spotlights = std::unique_ptr<SpotLightCollectionD3D11>(new SpotLightCollectionD3D11());
	_dirlights = std::unique_ptr<DirLightCollectionD3D11>(new DirLightCollectionD3D11());
	_pointlights = std::unique_ptr<PointLightCollectionD3D11>(new PointLightCollectionD3D11());

	_currCameraPtr = _camera.get();
}
Scene::~Scene()
{
}

bool Scene::Initialize(ID3D11Device *device, Content *content, Graphics *graphics)
{
	if (_initialized)
		return false;

	_device = device;
	_content = content;
	_graphics = graphics;

	// Create scene content holder
	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 15, 0), XMFLOAT3(16, 16, 16));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}


	// Create camera
	if (!_camera->Initialize(device,
		ProjectionInfo(70.0f * (XM_PI / 180.0f), 16.0f / 9.0f, 0.05f, 100.0f), 
		XMFLOAT4A(0.0f, 2.0f, -2.0f, 0.0f)))
	{
		ErrMsg("Failed to initialize camera!");
		return false;
	}
	
	// Create secondary camera
	if (!_secondaryCamera->Initialize(device,
		ProjectionInfo(70.0f * (XM_PI / 180.0f), 16.0f / 9.0f, 0.05f, 500.0f),
		XMFLOAT4A(0.0f, 2.0f, -2.0f, 0.0f)))
	{
		ErrMsg("Failed to initialize secondary camera!");
		return false;
	}


	// Create spotlights
	const SpotLightData spotlightInfo = {
		512,
		std::vector<SpotLightData::PerLightInfo> {
			SpotLightData::PerLightInfo {
				{ 4.0f, 2.5f, 0.0f },		// initialPosition
				{ 15.0f, 0.0f, 0.0f },		// color
				-XM_PIDIV2,					// rotationX
				0.0f,						// rotationY
				XM_PI * 0.5f,				// angle
				0.75f,						// falloff
				false,						// orthographic
				0.1f,						// projectionNearZ
				35.0f						// projectionFarZ
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 6.5f, 0.0f },		// initialPosition
				{ 0.0f, 15.0f, 0.0f },		// color
				0.0f,						// rotationX
				XM_PIDIV2,					// rotationY
				XM_PI * 0.5f,				// angle
				0.75f,						// falloff
				false,						// orthographic
				0.1f,						// projectionNearZ
				35.0f						// projectionFarZ
			},

			SpotLightData::PerLightInfo {
				{ 0.0f, 2.5f, 4.0f },		// initialPosition
				{ 0.0f, 0.0f, 15.0f },		// color
				XM_PI,						// rotationX
				0.0f,						// rotationY
				XM_PI * 0.5f,				// angle
				0.75f,						// falloff
				false,						// orthographic
				0.1f,						// projectionNearZ
				35.0f						// projectionFarZ
			},
		}
	};

	if (!_spotlights->Initialize(device, spotlightInfo))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}


	// Create directional lights
	const DirLightData dirlightInfo = {
		1024,
		std::vector<DirLightData::PerLightInfo> {
			DirLightData::PerLightInfo {
				{ 0.0475f, 0.0515f, 0.055f },	// color
				-0.746f,						// rotationX
				0.867f,							// rotationY
			},
		}
	};

	if (!_dirlights->Initialize(device, dirlightInfo))
	{
		ErrMsg("Failed to initialize directional light collection!");
		return false;
	}


	// Create pointlights
	const PointLightData pointlightInfo = {
		256,
		std::vector<PointLightData::PerLightInfo> {
			PointLightData::PerLightInfo {
				{ 7.0f, 5.0f, -9.0f },			// initialPosition
				{ 18.0f, 6.0f, 0.5f },			// color
				4.0f,							// falloff
				0.1f,							// projectionNearZ
				15.0f							// projectionFarZ
			},

			PointLightData::PerLightInfo {
				{ -5.0f, 3.0f, -5.5f },			// initialPosition
				{ 44.0f, 49.0f, 57.5f },		// color
				3.5f,							// falloff
				0.1f,							// projectionNearZ
				25.0f							// projectionFarZ
			},
		}
	};

	if (!_pointlights->Initialize(device, pointlightInfo))
	{
		ErrMsg("Failed to initialize pointlight collection!");
		return false;
	}


	// Create cubemap
	if (!_cubemap.Initialize(device, 128, 0.1f, 15.5f, { 0.0f, 15.0f, 0.0f, 0.0f }))
	{
		ErrMsg("Failed to initialize cubemap!");
		return false;
	}


	// Create global selection marker
	{
		const UINT
			meshID = content->GetMeshID("Mesh_WireframeCube"),
			textureID = content->GetTextureID("Tex_Red"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = content->GetTextureID("Tex_Red"),
			heightID = CONTENT_NULL;

        std::unique_ptr<Entity> entPtr = std::make_unique<Object>(-1, BoundingOrientedBox({ 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 }));
		Object *obj = reinterpret_cast<Object *>(entPtr.get());

		if (!obj->Initialize(_device, "Selection Marker", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize selection marker object!");
			return false;
		}

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global transform gizmo
	{
		const UINT
			meshID = content->GetMeshID("Mesh_TransformGizmo"),
			textureID = content->GetTextureID("Tex_TransformGizmo"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = content->GetTextureID("Tex_TransformGizmo"),
			heightID = CONTENT_NULL;

		std::unique_ptr<Entity> entPtr = std::make_unique<Object>(-1, BoundingOrientedBox({ 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 }));
		Object *obj = reinterpret_cast<Object *>(entPtr.get());

		if (!obj->Initialize(_device, "Transform Gizmo", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize transform gizmo object!");
			return false;
		}

		entPtr.get()->GetTransform()->SetScale({0.5f, 0.5f, 0.5f });

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global pointer gizmo
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Sphere"),
			textureID = content->GetTextureID("Tex_Black"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = content->GetTextureID("Tex_White"),
			heightID = CONTENT_NULL;

		std::unique_ptr<Entity> entPtr = std::make_unique<Object>(-1, BoundingOrientedBox({ 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 }));
		Object *obj = reinterpret_cast<Object *>(entPtr.get());

		if (!obj->Initialize(_device, "Pointer Gizmo", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize pointer object!");
			return false;
		}

		entPtr.get()->GetTransform()->SetScale({ 0.10f, 0.10f, 0.10f });

		_globalEntities.push_back(std::move(entPtr));
	}


	// Create room
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Plane"),
			textureID = content->GetTextureID("Tex_Metal"),
			normalID = content->GetTextureMapID("TexMap_Metal_Normal"),
			specularID = content->GetTextureMapID("TexMap_Metal_Specular"),
			reflectiveID = content->GetTextureMapID("TexMap_Metal_Reflective"),
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Room Floor", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize room floor object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });

		obj = reinterpret_cast<Object*>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Room Roof", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize room roof object!");
			return false;
		}

		reinterpret_cast<Entity*>(obj)->GetTransform()->SetPosition({ 0.0f, 30.0f, 0.0f });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetEuler({ 0.0f, 0.0f, XM_PI });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });


		obj = reinterpret_cast<Object*>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Room Wall South", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize room wall south object!");
			return false;
		}

		reinterpret_cast<Entity*>(obj)->GetTransform()->SetPosition({ 0.0f, 15.0f, -15.0f });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetEuler({ 0.5f * XM_PI, 0.0f, 0.0f });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });


		obj = reinterpret_cast<Object*>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Room Wall North", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize room wall north object!");
			return false;
		}

		reinterpret_cast<Entity*>(obj)->GetTransform()->SetPosition({ 0.0f, 15.0f, 15.0f });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetEuler({ -0.5f * XM_PI, 0.0f, 0.0f });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });


		obj = reinterpret_cast<Object*>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Room Wall West", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize room wall west object!");
			return false;
		}

		reinterpret_cast<Entity*>(obj)->GetTransform()->SetPosition({ -15.0f, 15.0f, 0.0f });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetEuler({ 0.0f, 0.0f, -0.5f * XM_PI });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });


		obj = reinterpret_cast<Object*>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Room Wall East", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize room wall east object!");
			return false;
		}

		reinterpret_cast<Entity*>(obj)->GetTransform()->SetPosition({ 15.0f, 15.0f, 0.0f });
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetEuler({ 0.0f, 0.0f, 0.5f * XM_PI});
		reinterpret_cast<Entity*>(obj)->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });
	}

	// Create model
	{
		const UINT
			meshID = content->GetMeshID("Mesh_CharacterSculptLow1"),
			textureID = content->GetTextureID("Tex_CharacterSculptLow0Texture1"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Character", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize model object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ 0.0f, 0.0f, 0.0f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetEuler({ 0.0f, XM_PI, 0.0f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetScale({ 0.3f, 0.3f, 0.3f });
	}

	// Create reflective sphere
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Sphere"),
			textureID = content->GetTextureID("Tex_White"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = content->GetTextureMapID("TexMap_White_Reflective"),
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Reflective Sphere", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize reflective sphere object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ 0.0f, 15.0f, 0.0f });
	}

	// Create submesh
	{
		const UINT
			meshID = content->GetMeshID("Mesh_SimpleSubmesh"),
			textureID = content->GetTextureID("Tex_texture3"),
			normalID = content->GetTextureMapID("TexMap_texture3_Normal"),
			specularID = content->GetTextureMapID("TexMap_Default_Specular"),
			reflectiveID = content->GetTextureMapID("TexMap_Default_Reflective"),
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Submesh", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize submesh object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ -5.0f, 10.0f, 5.0f });
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

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "PBR Sphere", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize PBR object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ 6.0f, 2.5f, -7.5f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });
	}

	// Create error
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Error"),
			textureID = content->GetTextureID("Tex_White"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = content->GetTextureID("Tex_Red"),
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "ERROR", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize error object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ -4.0f, 3.0f, 7.0f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetEuler({ 0.0f, -XM_PIDIV2, 0.0f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetScale({ 1.2f, 1.2f, 1.2f });
	}

	// Create transparent
	{
		const UINT
			meshID = content->GetMeshID("Mesh_Fallback"),
			textureID = content->GetTextureID("Tex_Transparent"),
			normalID = content->GetTextureMapID("TexMap_Bricks_Normal"),
			specularID = content->GetTextureMapID("TexMap_Bricks_Specular"),
			reflectiveID = CONTENT_NULL,
			ambientID = CONTENT_NULL,
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Transparent", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID, true))
		{
			ErrMsg("Failed to initialize transparent object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ 2.0f, 1.5f, 4.0f });
	}

	// Create parent
	{
		const UINT
			meshID = content->GetMeshID("Mesh_CharacterSculptLow1"),
			textureID = content->GetTextureID("Tex_CharacterSculptLow0Texture1"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, "Parent", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize parent object!");
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ -7.5f, 0.0f, -7.0f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetEuler({ 0.0f, 0.5f * XM_PI, 0.0f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetScale({ 0.3f, 0.3f, 0.3f });
	}

	std::string nextParent = "Parent";
	std::string nextChild = "Child 1";
	for (int i = 0; i < 32; i++)
	{
		// Create child i
		const UINT
			meshID = content->GetMeshID("Mesh_CharacterSculptLow1"),
			textureID = content->GetTextureID("Tex_CharacterSculptLow0Texture1"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = content->GetTextureID("Tex_Ambient"),
			heightID = CONTENT_NULL;

		Object *obj = reinterpret_cast<Object *>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
		if (!obj->Initialize(_device, nextChild, meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg(std::format("Failed to initialize {} object!", nextChild));
			return false;
		}

		reinterpret_cast<Entity *>(obj)->GetTransform()->SetPosition({ 1.0f, 0.0f, 2.5f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetEuler({ 0.0f, -0.25f, 0.0f });
		reinterpret_cast<Entity *>(obj)->GetTransform()->SetScale({ 0.9f, 0.9f, 0.9f });

		reinterpret_cast<Entity *>(obj)->SetParent(_sceneHolder.GetEntityByName(nextParent), true);

		nextParent = nextChild;
		nextChild = std::format("Child {}", i + 2);
	}


	// Create emitter
	{
		Emitter *emitter = reinterpret_cast<Emitter *>(_sceneHolder.AddEntity(DirectX::BoundingOrientedBox({ 0,0,0 }, { 1,1,1 }, { 0,0,0,1 }), EntityType::EMITTER));

		EmitterData emitterData = { };
		emitterData.particleCount = 1024;	
		emitterData.particleRate = 1;
		emitterData.lifetime = 5.0f;

		if (!emitter->Initialize(_device, "Dust", emitterData, content->GetTextureID("Tex_Particle")))
		{
			ErrMsg("Failed to initialize emitter!");
			return false;
		}

		reinterpret_cast<Entity *>(emitter)->GetTransform()->SetPosition({ 0.0f, 15.0f, 0.0f });
	}
	
	_initialized = true;
	return true;
}

bool Scene::Update(ID3D11DeviceContext* context, Time& time, const Input& input)
{
	if (!_initialized)
		return false;

	if (_rotateLights)
	{
		_spotlights->GetLightCamera(0)->LookY(time.deltaTime * 0.5f);
		_spotlights->GetLightCamera(0)->MoveUp(time.deltaTime * 2.0f);

		_spotlights->GetLightCamera(1)->LookY(time.deltaTime * 0.5f);
		_spotlights->GetLightCamera(1)->MoveUp(time.deltaTime * 2.0f);

		_spotlights->GetLightCamera(2)->LookX(time.deltaTime * 0.5f);
		_spotlights->GetLightCamera(2)->MoveRight(time.deltaTime * -2.0f);
	}

	if (!UpdatePlayer(context, time, input))
	{
		ErrMsg("Failed to update player!");
		return false;
	}

	if (!UpdateEntities(context, time, input))
	{
		ErrMsg("Failed to update scene entities!");
		return false;
	}

	return true;
}

bool Scene::UpdatePlayer(ID3D11DeviceContext* context, Time& time, const Input& input)
{
	if (input.IsInFocus()) // Handle user input while window is in focus
	{
		if (input.GetKey(KeyCode::K) == KeyState::Pressed)
			_playerPhysics = !_playerPhysics;
	}

	if (_playerPhysics)
	{
		if (!UpdatePhysicsPlayer(context, time, input))
		{
			ErrMsg("Failed to update physics player!");
			return false;
		}
	}
	else
	{
		if (!UpdateDebugPlayer(context, time, input))
		{
			ErrMsg("Failed to update debug player!");
			return false;
		}
	}

	return true;
}
bool Scene::UpdatePhysicsPlayer(ID3D11DeviceContext* context, Time& time, const Input& input)
{
	static float playerHeight = 3.3f;
	static float playerVelocity = 0.0f;
	static float playerGravity = -19.82f;

	static float playerSpeed = 4.0f;
	static float playerJump = 10.0f;

	static bool isGrounded = false;


	CameraD3D11* basisCamera = _useMainCamera ? _camera.get() : _currCameraPtr;

	if (input.IsInFocus()) // Handle user input while window is in focus
	{
		DirectX::XMFLOAT4A playerForward = basisCamera->GetForward();
		playerForward.y = 0.0f;
		*reinterpret_cast<XMVECTOR*>(&(playerForward)) = XMVector3Normalize(*reinterpret_cast<XMVECTOR*>(&(playerForward)));

		// Move camera
		if (input.GetKey(KeyCode::D) == KeyState::Held)
			basisCamera->MoveRight(time.deltaTime * playerSpeed);
		else if (input.GetKey(KeyCode::A) == KeyState::Held)
			basisCamera->MoveRight(-time.deltaTime * playerSpeed);

		if (input.GetKey(KeyCode::W) == KeyState::Held)
			basisCamera->Move(time.deltaTime * playerSpeed, playerForward);
		else if (input.GetKey(KeyCode::S) == KeyState::Held)
			basisCamera->Move(-time.deltaTime * playerSpeed, playerForward);

		if (isGrounded && input.GetKey(KeyCode::Space) == KeyState::Pressed)
		{
			playerVelocity = playerJump; // Jump
			isGrounded = false;
		}

		const MouseState mState = input.GetMouse();
		if (mState.dx != 0) basisCamera->LookX(static_cast<float>(mState.dx) / 360.0f);
		if (mState.dy != 0) basisCamera->LookY(static_cast<float>(mState.dy) / 360.0f);
	}

	float playerY = basisCamera->GetTransform().GetPosition().y;
	if (playerY > playerHeight)
	{
		playerVelocity += playerGravity * time.deltaTime;
		isGrounded = false;
	}
	else
	{
		isGrounded = true;

		if (playerVelocity < 0.0f)
			playerVelocity = 0.0f;

		if (playerY < playerHeight)
		{
			basisCamera->Move(playerHeight - playerY, { 0.0f, 1.0f, 0.0f, 0.0f });
		}
	}

	if (playerVelocity != 0.0f)
	{
		_currCameraPtr->Move(playerVelocity * time.deltaTime, { 0.0f, 1.0f, 0.0f, 0.0f });
	}

	return true;
}
bool Scene::UpdateDebugPlayer(ID3D11DeviceContext* context, Time& time, const Input& input)
{
	if (input.IsInFocus()) // Handle user input while window is in focus
	{
		if (input.GetKey(KeyCode::G) == KeyState::Pressed)
			_rotateLights = !_rotateLights;

		if (input.GetKey(KeyCode::Z) == KeyState::Pressed)
			_useMainCamera = !_useMainCamera;
		else if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			_useMainCamera = true;

		CameraD3D11* basisCamera = _useMainCamera ? _camera.get() : _currCameraPtr;

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

		static UINT transparentStart = _content->GetTextureID("Tex_Transparent");
		static UINT ambientID = _content->GetTextureID("Tex_Ambient");

		if (input.GetKey(KeyCode::P) == KeyState::Pressed) // Create random entities within the scene bounds
		{
			size_t count = 10;

			if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
				count = 50;
			
			if (input.GetKey(KeyCode::LeftControl) == KeyState::Held)
				count *= 0.1f;

			const BoundingBox sceneBounds = _sceneHolder.GetBounds();
			const XMFLOAT3
				sceneCenter = sceneBounds.Center,
				sceneExtents = sceneBounds.Extents;

			for (size_t i = 0; i < count; i++)
			{
				const UINT
					meshID = rand() % _content->GetMeshCount(),
					textureID = rand() % _content->GetTextureCount();

				Object *obj = reinterpret_cast<Object*>(_sceneHolder.AddEntity(_content->GetMesh(meshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
				if (!obj->Initialize(_device, "Random Entity",
					meshID, textureID,
					CONTENT_NULL, CONTENT_NULL,
					CONTENT_NULL, ambientID,
					CONTENT_NULL,
					(textureID >= transparentStart)))
				{
					ErrMsg(std::format("Failed to initialize entity #{}!", reinterpret_cast<Entity*>(obj)->GetID()));
					return false;
				}

				reinterpret_cast<Entity*>(obj)->GetTransform()->SetPosition({
					sceneCenter.x + sceneExtents.x * static_cast<float>((rand() % 2000) - 1000) / 1000.0f,
					sceneCenter.y + sceneExtents.y * static_cast<float>((rand() % 2000) - 1000) / 1000.0f,
					sceneCenter.z + sceneExtents.z * static_cast<float>((rand() % 2000) - 1000) / 1000.0f
				});

				reinterpret_cast<Entity*>(obj)->GetTransform()->SetEuler({
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f),
					static_cast<float>((rand() % 2000)) * (XM_2PI / 2000.0f)
				});

				if (_currSelection >= 0)
				{
					reinterpret_cast<Entity*>(obj)->SetParent(_sceneHolder.GetEntity(_currSelection));
				}
			}
		}
		else if (input.GetKey(KeyCode::O) == KeyState::Pressed)
		{ // Create one custom entity in front of the camera
			Object* obj = reinterpret_cast<Object*>(_sceneHolder.AddEntity(_content->GetMesh(selectedMeshID)->GetBoundingOrientedBox(), EntityType::OBJECT));
			if (!obj->Initialize(_device, "Custom Entity",
				selectedMeshID, selectedTextureID,
				CONTENT_NULL, CONTENT_NULL,
				CONTENT_NULL, ambientID,
				CONTENT_NULL,
				(selectedTextureID >= transparentStart)))
			{
				ErrMsg(std::format("Failed to initialize entity #{}!", reinterpret_cast<Entity*>(obj)->GetID()));
				return false;
			}

			XMFLOAT4A camForward = basisCamera->GetForward();
			XMFLOAT4A pos = basisCamera->GetPosition();
			*reinterpret_cast<XMVECTOR*>(&camForward) *= 3.0f;
			*reinterpret_cast<XMVECTOR*>(&camForward) += *reinterpret_cast<const XMVECTOR*>(&pos);

			reinterpret_cast<Entity*>(obj)->GetTransform()->SetPosition(camForward);
			reinterpret_cast<Entity*>(obj)->GetTransform()->SetRotation(basisCamera->GetTransform().GetRotation());

			if (_currSelection >= 0)
			{
				reinterpret_cast<Entity*>(obj)->SetParent(_sceneHolder.GetEntity(_currSelection));
			}
			else
			{
				_currSelection = static_cast<int>(_sceneHolder.GetEntityCount()) - 1;
			}
		}

		if (input.GetKey(KeyCode::M2) == KeyState::Pressed)
		{
			if (_currSelection != -1)
			{
				_currSelection = -1;
			}
			else
			{
				const XMFLOAT4A
					camPos = basisCamera->GetPosition(),
					camDir = basisCamera->GetForward();

				RaycastOut out;
				if (_sceneHolder.Raycast(
					{ camPos.x, camPos.y, camPos.z },
					{ camDir.x, camDir.y, camDir.z },
					out))
				{
					const UINT entityI = _sceneHolder.GetEntityIndex(out.entity);
					_currSelection = (entityI == 0xffffffff) ? -1 : static_cast<int>(entityI);
				}
				else
					_currSelection = -1;
			}
		}

		if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			_currSelection = -1;

		static bool movePointLights = false;

		if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			movePointLights = false;
		else if (input.GetKey(KeyCode::F) == KeyState::Pressed)
		{
			movePointLights = !movePointLights;
			_currSelection = movePointLights ? 0 : -1;
		}

		if (input.IsCursorLocked())
		{
			if (input.GetKey(KeyCode::Add) == KeyState::Pressed)
				_currSelection++;
			if (input.GetKey(KeyCode::Subtract) == KeyState::Pressed)
				_currSelection--;

			if (_currSelection < -1)
				_currSelection = -1;
			else if (_currSelection >= static_cast<int>(_sceneHolder.GetEntityCount()))
				_currSelection = static_cast<int>(_sceneHolder.GetEntityCount()) - 1;

			float currSpeed = 3.5f;
			if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
				currSpeed = 15.0f;
			if (input.GetKey(KeyCode::LeftControl) == KeyState::Held)
				currSpeed = 0.5f;

			if (_currSelection == -1 && !movePointLights) // Move camera
			{
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
			else // Move selected entity or point light
			{
				static bool relativeToCamera = false;
				if (input.GetKey(KeyCode::M) == KeyState::Pressed)
					relativeToCamera = !relativeToCamera;

				XMVECTOR right, up, forward;

				if (relativeToCamera)
				{
					XMFLOAT4A r, u, f;
					basisCamera->GetAxes(&r, &u, &f);

					right = XMLoadFloat4A(&r);
					up = XMLoadFloat4A(&u);
					forward = XMLoadFloat4A(&f);
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

				if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
				{
					relativeToCamera = false;
					isRotating = false;
					isScaling = false;
				}

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
					if (movePointLights)
					{
						_currSelection = std::clamp(_currSelection, 0, static_cast<int>(_pointlights->GetNrOfLights()) - 1);
						_pointlights->Move(_currSelection, *reinterpret_cast<XMFLOAT4A*>(&transformationVector));
					}
					else
					{
						Entity* ent = _sceneHolder.GetEntity(_currSelection);
						Transform* entityTransform = ent->GetTransform();

						if (isRotating)
							entityTransform->Rotate(*reinterpret_cast<XMFLOAT4A*>(&transformationVector), _editSpace);
						else if (isScaling)
							entityTransform->Scale(*reinterpret_cast<XMFLOAT4A*>(&transformationVector), _editSpace);
						else
							entityTransform->Move(*reinterpret_cast<XMFLOAT4A*>(&transformationVector), _editSpace);

						if (!_sceneHolder.UpdateEntityPosition(ent))
						{
							ErrMsg("Failed to update entity position!");
							return false;
						}
					}
				}
			}

			if (_currSelection >= 0)
			{
				if (input.GetKey(KeyCode::Delete) == KeyState::Pressed)
				{
					const Entity* ent = _sceneHolder.GetEntity(_currSelection);
					if (!_sceneHolder.RemoveEntity(_currSelection))
					{
						ErrMsg("Failed to remove entity!");
						delete ent;
						return false;
					}
					delete ent;
					_currSelection = -1;
				}
			}
		
			if (input.GetKey(KeyCode::M1) == KeyState::Held)
			{
				_drawPointer = true;
			}
		}

		static bool hasSetCamera = false;
		if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			_currCamera = -3;

		if (!hasSetCamera)
		{
			_currCameraPtr = _camera.get();
			if (!_graphics->SetSpotlightCollection(_spotlights.get()))
			{
				ErrMsg("Failed to set spotlight collection!");
				return false;
			}

			if (!_graphics->SetDirlightCollection(_dirlights.get()))
			{
				ErrMsg("Failed to set directional light collection!");
				return false;
			}

			if (!_graphics->SetPointlightCollection(_pointlights.get()))
			{
				ErrMsg("Failed to set pointlight collection!");
				return false;
			}

			hasSetCamera = true;
		}
		else if (input.GetKey(KeyCode::C) == KeyState::Pressed || _currCamera == -3)
		{ // Change camera
			_currCamera++;

			const int
				spotlightCount = static_cast<int>(_spotlights->GetNrOfLights()),
				dirlightCount = static_cast<int>(_dirlights->GetNrOfLights());

			if (_currCamera - 6 - spotlightCount - dirlightCount >= 0)
				_currCamera = -2;

			if (_currCamera < 0)
			{
				if (_currCamera == -1)
					_currCameraPtr = _secondaryCamera.get();
				else
				{
					_currCamera = -2;
					_currCameraPtr = _camera.get();
				}
			}
			else if (_currCamera < 6)
				_currCameraPtr = _cubemap.GetCamera(_currCamera);
			else if (_currCamera - 6 < spotlightCount)
				_currCameraPtr = _spotlights->GetLightCamera(_currCamera - 6);
			else if (_currCamera - 6 - spotlightCount < dirlightCount)
				_currCameraPtr = _dirlights->GetLightCamera(_currCamera - 6 - spotlightCount);
		}
	}

	UpdateGlobalEntities();

	return true;
}

bool Scene::UpdateEntities(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	if (!_camera->UpdateBuffers(context))
	{
		ErrMsg("Failed to update camera buffers!");
		return false;
	}

	if (!_secondaryCamera->UpdateBuffers(context))
	{
		ErrMsg("Failed to update secondary camera buffers!");
		return false;
	}

	if (!_spotlights->UpdateBuffers(context))
	{
		ErrMsg("Failed to update spotlight buffers!");
		return false;
	}

	BoundingBox cubemapBounds;
	if (_cubemap.GetUpdate())
		_cubemap.StoreBounds(cubemapBounds);

	if (!_dirlights->ScaleToScene(*_camera, _sceneHolder.GetBounds(), _cubemap.GetUpdate() ? &cubemapBounds : nullptr))
	{
		ErrMsg("Failed to scale directional lights to scene & camera!");
		return false;
	}

	if (!_dirlights->UpdateBuffers(context))
	{
		ErrMsg("Failed to update directional light buffers!");
		return false;
	}

	if (!_pointlights->UpdateBuffers(context))
	{
		ErrMsg("Failed to update pointlight buffers!");
		return false;
	}

	if (_graphics->GetUpdateCubemap())
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

	const UINT globalEntityCount = _globalEntities.size();
	for (UINT i = 0; i < globalEntityCount; i++)
	{
		Entity *ent = _globalEntities.at(i).get();

		if (!ent->Update(context, time, input))
		{
			ErrMsg(std::format("Failed to update global entity #{}!", i));
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
void Scene::UpdateGlobalEntities()
{
	Entity
		*selection = _currSelection < 0 ? nullptr : _sceneHolder.GetEntity(_currSelection),
		*marker = nullptr,
		*gizmo = nullptr,
		*pointer = nullptr;

	for (auto &entPtr : _globalEntities)
	{
		if (entPtr->GetName() == "Selection Marker")
		{
			marker = entPtr.get();
		}
		else if (entPtr->GetName() == "Transform Gizmo")
		{
			gizmo = entPtr.get();
		}
		else if (entPtr->GetName() == "Pointer Gizmo")
		{
			pointer = entPtr.get();
		}
	}

	if (selection)
	{
		if (marker)
		{
			BoundingOrientedBox box = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0, 1} };
			selection->StoreBounds(box);

			const XMFLOAT4A
				center = { box.Center.x, box.Center.y, box.Center.z, 0 },
				extents = { box.Extents.x, box.Extents.y, box.Extents.z, 0 },
				rot = { box.Orientation.x, box.Orientation.y, box.Orientation.z, box.Orientation.w };

			marker->GetTransform()->SetPosition(center, World);
			marker->GetTransform()->SetRotation(rot, World);
			marker->GetTransform()->SetScale(extents, World);
		}

		if (gizmo)
		{
			gizmo->GetTransform()->SetPosition(selection->GetTransform()->GetPosition(World), World);
			gizmo->GetTransform()->SetRotation(selection->GetTransform()->GetRotation(World), World);
		}
	}

	if (_drawPointer)
	{
		if (pointer)
		{
			const XMFLOAT3A
				camPos = _camera->GetTransform().GetPosition(),
				camDir = _camera->GetTransform().GetForward();

			RaycastOut out;
			if (_sceneHolder.Raycast(
				{ camPos.x, camPos.y, camPos.z },
				{ camDir.x, camDir.y, camDir.z },
				out))
			{
				//out.distance -= 0.05f;

				XMFLOAT3A pos = { 
					camPos.x + (camDir.x * out.distance),
					camPos.y + (camDir.y * out.distance),
					camPos.z + (camDir.z * out.distance)
				};

				pointer->GetTransform()->SetPosition(pos, World);

				/*
				//XMFLOAT4A rot = _camera->GetTransform().GetRotation();
				//XMStoreFloat4A(&rot, XMQuaternionInverse(XMLoadFloat4A(&rot)));

				XMMATRIX lookAtMatrix = DirectX::XMMatrixLookAtLH(XMLoadFloat3A(&pos), XMLoadFloat3A(&camPos), XMVectorSet(0, 1, 0, 0));
				XMFLOAT4A rot = { 0, 0, 0, 1 };
				XMStoreFloat4A(&rot, XMQuaternionRotationMatrix(lookAtMatrix));

				pointer->GetTransform()->SetRotation(rot, World);
				*/
			}
			else
			{
				_drawPointer = false;
			}
		}
		else
		{
			_drawPointer = false;
		}
	}
}

bool Scene::Render(Time &time, const Input &input)
{
	if (!_initialized)
		return false;

	if (!_graphics->SetCameras(_camera.get(), _currCameraPtr))
	{
		ErrMsg("Failed to set camera!");
		return false;
	}

	std::vector<Entity *> entitiesToRender;
	entitiesToRender.reserve(_camera->GetCullCount());

	union {
		BoundingFrustum frustum = {};
		BoundingOrientedBox box;
	} view;
	bool isCameraOrtho = _camera->GetOrtho();

	time.TakeSnapshot("FrustumCull");
	if (isCameraOrtho)
	{
		if (!_camera->StoreBounds(view.box))
		{
			ErrMsg("Failed to store camera box!");
			return false;
		}

		if (!_sceneHolder.BoxCull(view.box, entitiesToRender))
		{
			ErrMsg("Failed to perform box culling!");
			return false;
		}
	}
	else
	{
		if (!_camera->StoreBounds(view.frustum))
		{
			ErrMsg("Failed to store camera frustum!");
			return false;
		}

		if (!_sceneHolder.FrustumCull(view.frustum, entitiesToRender))
		{
			ErrMsg("Failed to perform frustum culling!");
			return false;
		}
	}

	for (Entity *ent : entitiesToRender)
	{
		if (!ent->Render(_camera.get()))
		{
			ErrMsg("Failed to render entity!");
			return false;
		}
	}
	time.TakeSnapshot("FrustumCull");

	if (!RenderGlobal())
	{
		ErrMsg("Failed to render global entities!");
		return false;
	}
	
	const int spotlightCount = static_cast<int>(_spotlights->GetNrOfLights());
	time.TakeSnapshot("FrustumCullSpotlights");
	if (_doMultiThread)
		#pragma omp parallel for num_threads(2) // TODO: Due to transform batching, this may no longer be thread safe
		for (int i = 0; i < spotlightCount; i++)
		{
			CameraD3D11 *spotlightCamera = _spotlights->GetLightCamera(i);

			std::vector<Entity *> entitiesToCastShadows;
			entitiesToCastShadows.reserve(spotlightCamera->GetCullCount());

			bool isSpotlightOrtho = spotlightCamera->GetOrtho();

			bool intersectResult = _graphics->GetUpdateCubemap() && _cubemap.GetUpdate();
			if (isSpotlightOrtho)
			{
				BoundingOrientedBox lightBounds;
				if (!spotlightCamera->StoreBounds(lightBounds))
				{
					ErrMsg("Failed to store spotlight camera oriented box!");
					continue;
				}

				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(lightBounds);
				else				intersectResult = intersectResult || view.frustum.Intersects(lightBounds);

				if (!intersectResult)
				{ // Skip rendering if the bounds don't intersect
					_spotlights->SetLightEnabled(i, false);
					continue;
				}

				if (!_sceneHolder.BoxCull(lightBounds, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform box culling for spotlight #{}!", i));
					continue;
				}
			}
			else
			{
				BoundingFrustum lightBounds;
				if (!spotlightCamera->StoreBounds(lightBounds))
				{
					ErrMsg("Failed to store spotlight camera frustum!");
					continue;
				}

				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(lightBounds);
				else				intersectResult = intersectResult || view.frustum.Intersects(lightBounds);

				if (!intersectResult)
				{ // Skip rendering if the bounds don't intersect
					_spotlights->SetLightEnabled(i, false);
					continue;
				}
				_spotlights->SetLightEnabled(i, true);

				if (!_sceneHolder.FrustumCull(lightBounds, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform frustum culling for spotlight #{}!", i));
					continue;
				}
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

			bool isSpotlightOrtho = spotlightCamera->GetOrtho();

			bool intersectResult = _graphics->GetUpdateCubemap() && _cubemap.GetUpdate();
			if (isSpotlightOrtho)
			{
				BoundingOrientedBox lightBounds;
				if (!spotlightCamera->StoreBounds(lightBounds))
				{
					ErrMsg("Failed to store spotlight camera oriented box!");
					return false;
				}

				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(lightBounds);
				else				intersectResult = intersectResult || view.frustum.Intersects(lightBounds);

				if (!intersectResult)
				{ // Skip rendering if the bounds don't intersect
					_spotlights->SetLightEnabled(i, false);
					continue;
				}

				time.TakeSnapshot(std::format("FrustumCullSpotlight{}", i));
				if (!_sceneHolder.BoxCull(lightBounds, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform box culling for spotlight #{}!", i));
					return false;
				}
				time.TakeSnapshot(std::format("FrustumCullSpotlight{}", i));
			}
			else
			{
				BoundingFrustum lightBounds;
				if (!spotlightCamera->StoreBounds(lightBounds))
				{
					ErrMsg("Failed to store spotlight camera frustum!");
					return false;
				}

				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(lightBounds);
				else				intersectResult = intersectResult || view.frustum.Intersects(lightBounds);

				if (!intersectResult)
				{ // Skip rendering if the bounds don't intersect
					_spotlights->SetLightEnabled(i, false);
					continue;
				}
				_spotlights->SetLightEnabled(i, true);

				time.TakeSnapshot(std::format("FrustumCullSpotlight{}", i));
				if (!_sceneHolder.FrustumCull(lightBounds, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform frustum culling for spotlight #{}!", i));
					return false;
				}
				time.TakeSnapshot(std::format("FrustumCullSpotlight{}", i));
			}

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
	
	const int dirlightCount = static_cast<int>(_dirlights->GetNrOfLights());
	time.TakeSnapshot("FrustumCullDirlights");
	#pragma omp parallel for num_threads(2) // TODO: Due to transform batching, this may no longer be thread safe
	for (int i = 0; i < dirlightCount; i++)
	{
		CameraD3D11 *dirlightCamera = _dirlights->GetLightCamera(i);

		std::vector<Entity *> entitiesToCastShadows;
		entitiesToCastShadows.reserve(dirlightCamera->GetCullCount());

		bool intersectResult = _graphics->GetUpdateCubemap() && _cubemap.GetUpdate();
		BoundingOrientedBox lightBounds;
		if (!dirlightCamera->StoreBounds(lightBounds))
		{
			ErrMsg("Failed to store directional light camera oriented box!");
			continue;
		}

		if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(lightBounds);
		else				intersectResult = intersectResult || view.frustum.Intersects(lightBounds);

		if (!intersectResult)
		{ // Skip rendering if the bounds don't intersect
			_dirlights->SetLightEnabled(i, false);
			continue;
		}

		if (!_sceneHolder.BoxCull(lightBounds, entitiesToCastShadows))
		{
			ErrMsg(std::format("Failed to perform box culling for directional light #{}!", i));
			continue;
		}

		for (Entity *ent : entitiesToCastShadows)
		{
			if (!ent->Render(dirlightCamera))
			{
				ErrMsg(std::format("Failed to render entity for directional light #{}!", i));
				break;
			}
		}
	}
	time.TakeSnapshot("FrustumCullDirlights");

	const int pointlightCount = static_cast<int>(_pointlights->GetNrOfLights());
	time.TakeSnapshot("FrustumCullPointlights");
	if (_doMultiThread)
		#pragma omp parallel for num_threads(2) // TODO: Due to transform batching, this may no longer be thread safe
		for (int i = 0; i < pointlightCount; i++)
			for (int j = 0; j < 6; j++)
			{
				CameraD3D11 *pointlightCamera = _pointlights->GetLightCamera(i, j);

				std::vector<Entity *> entitiesToCastShadows;
				entitiesToCastShadows.reserve(pointlightCamera->GetCullCount());

				BoundingFrustum pointlightFrustum;
				if (!pointlightCamera->StoreBounds(pointlightFrustum))
				{
					ErrMsg("Failed to store pointlight camera frustum!");
					continue;
				}

				bool intersectResult = _graphics->GetUpdateCubemap() && _cubemap.GetUpdate();
				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(pointlightFrustum);
				else				intersectResult = intersectResult || view.frustum.Intersects(pointlightFrustum);

				if (!intersectResult)
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

				BoundingFrustum pointlightFrustum;
				if (!pointlightCamera->StoreBounds(pointlightFrustum))
				{
					ErrMsg("Failed to store pointlight camera frustum!");
					return false;
				}

				bool intersectResult = _graphics->GetUpdateCubemap() && _cubemap.GetUpdate();
				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(pointlightFrustum);
				else				intersectResult = intersectResult || view.frustum.Intersects(pointlightFrustum);

				if (!intersectResult)
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
	if (_graphics->GetUpdateCubemap() && _cubemap.GetUpdate())
	{
		if (_doMultiThread)
			#pragma omp parallel for num_threads(2) // TODO: Due to transform batching, this may no longer be thread safe
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

				if (!cubemapCamera->StoreBounds(view.frustum)) // using predefined view frustum to save memory :}
				{
					ErrMsg("Failed to store cubemap camera frustum!");
					return false;
				}

				if (!_sceneHolder.FrustumCull(view.frustum, entitiesToRender))
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

	if (!_graphics->SetCubemap(&_cubemap))
	{
		ErrMsg("Failed to set cubemap!");
		return false;
	}

	return true;
}
bool Scene::RenderGlobal()
{
	for (auto &entPtr : _globalEntities)
	{
		Entity *ent = entPtr.get();

		if (_currSelection >= 0) // Render selection marker and transform gizmo
		{
			if (ent->GetName() == "Selection Marker" || 
				ent->GetName() == "Transform Gizmo")
			{
				if (!ent->Render(_camera.get()))
				{
					ErrMsg("Failed to render entity!");
					return false;
				}
			}
		}

		if (_drawPointer) // Render pointer
		{
			if (ent->GetName() == "Pointer Gizmo")
			{
				if (!ent->Render(_camera.get()))
				{
					ErrMsg("Failed to render entity!");
					return false;
				}
				
				_drawPointer = false;
			}
		}

		if (ent->GetName() == "Culling Tree Wireframe" || 
			ent->GetName() == "Entity Bounds Wireframe")
		{
			if (!ent->Render(_camera.get()))
			{
				ErrMsg("Failed to render entity!");
				return false;
			}
		}
	}

	return true;
}

bool Scene::RenderUI()
{
	ImGui::Text(std::format("Objects in scene: {}", _sceneHolder.GetEntityCount()).c_str());
		
	if (ImGui::TreeNode("Scene Hierarchy"))
	{
		ImGuiChildFlags childFlags = 0;
		childFlags |= ImGuiChildFlags_Border;
		childFlags |= ImGuiChildFlags_ResizeY;

		ImGui::BeginChild("Scene Hierarchy", ImVec2(0, 300), childFlags);

		std::vector<Entity*> sceneContent;
		_sceneHolder.GetEntities(sceneContent);

		for (auto& entity : sceneContent)
		{
			if (entity->GetParent() != nullptr) // Skip non-root entities
				continue;

			std::string entName = entity->GetName();
			UINT entIndex = _sceneHolder.GetEntityIndex(entity);

			if (_currSelection >= 0)
				if (entIndex == _currSelection)
					entName = std::format("<{}>", entName);

			if (ImGui::SmallButton(std::format("{} ({})", entName, entIndex).c_str()))
				_currSelection = entIndex;

			if (!RenderHierarchyUI(entity, 0))
			{
				ImGui::EndChild();
				ImGui::TreePop();
				return false;
			}
		}

		ImGui::EndChild();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Entity Hierarchy"))
	{
		ImGuiChildFlags childFlags = 0;
		childFlags |= ImGuiChildFlags_Border;
		childFlags |= ImGuiChildFlags_ResizeY;

		ImGui::BeginChild("Entity Hierarchy", ImVec2(0, 300), childFlags);
		if (_currSelection >= 0)
		{
			Entity *ent = _sceneHolder.GetEntity(_currSelection);

			ImGui::Text(std::format("<{}> ({})", ent->GetName(), _currSelection).c_str());
			if (!RenderHierarchyUI(ent, 0))
			{
				ImGui::EndChild();
				ImGui::TreePop();
				return false;
			}
		}
		ImGui::EndChild();

		ImGui::TreePop();
	}

	if (!RenderSelectionUI())
		return false;

	ImGui::Separator();

	if (ImGui::Button("Generate entity bounds"))
		DebugGenerateEntityBounds();

	if (ImGui::Button("Generate volume tree structure"))
		DebugGenerateVolumeTreeStructure();

	if (ImGui::Button(_doMultiThread ? "Threading On" : "Threading Off"))
		_doMultiThread = !_doMultiThread;

	bool isOrtho = _camera->GetOrtho();
	if (ImGui::Button(isOrtho ? "Orthographic: true" : "Orthographic: false"))
	{
		isOrtho = !isOrtho;
		_camera->SetOrtho(isOrtho);
	}

	static char fovText[16] = "90.0";
	ImGuiInputTextFlags textInputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll;
	ImGui::SetNextItemWidth(128);
	if (isOrtho)
	{
		if (ImGui::InputText("View Width", fovText, 16, textInputFlags))
		{
			float newWidth = static_cast<float>(atof(fovText));
			if (newWidth > 0.0f)
				_camera->SetFOV(newWidth);
		}
	}
	else
	{
		if (ImGui::InputText("FOV", fovText, 16, textInputFlags))
		{
			float newFOV = static_cast<float>(atof(fovText));
			if (newFOV > 0.0f && newFOV < 180.0f)
				_camera->SetFOV(newFOV * (XM_PI / 180.0f));
		}
	}

	XMFLOAT4A camPos = _camera->GetPosition();
	char camXCoord[32]{}, camYCoord[32]{}, camZCoord[32]{};
	snprintf(camXCoord, sizeof(camXCoord), "%.2f", camPos.x);
	snprintf(camYCoord, sizeof(camYCoord), "%.2f", camPos.y);
	snprintf(camZCoord, sizeof(camZCoord), "%.2f", camPos.z);
	ImGui::Text(std::format("Main Cam pos: ({}, {}, {})", camXCoord, camYCoord, camZCoord).c_str());

	if (_currCameraPtr != _camera.get() && _currCameraPtr != nullptr)
	{
		camPos = _currCameraPtr->GetPosition();
		snprintf(camXCoord, sizeof(camXCoord), "%.2f", camPos.x);
		snprintf(camYCoord, sizeof(camYCoord), "%.2f", camPos.y);
		snprintf(camZCoord, sizeof(camZCoord), "%.2f", camPos.z);
		ImGui::Text(std::format("Virt Cam pos: ({}, {}, {})", camXCoord, camYCoord, camZCoord).c_str());
	}

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
bool Scene::RenderSelectionUI()
{
	ImGui::Separator();
	ImGui::Text(std::format("Selected Index: {}", _currSelection).c_str());

	if (_currSelection < 0)
		return true;
	
	Entity *ent = _sceneHolder.GetEntity(_currSelection);

	if (!ent)
	{
		ImGui::Text("Entity: NULL");
		return true;
	}

	ImGui::Text(std::format("Entity: {}", ent->GetName()).c_str()); 
	
	Entity *parent = ent->GetParent();
	if (parent)
	{
		UINT parentIndex = _sceneHolder.GetEntityIndex(parent);

		ImGui::Text("Parent: ");
		ImGui::SameLine();
		if (ImGui::SmallButton(std::format("{} ({})", parent->GetName(), parentIndex).c_str()))
			_currSelection = parentIndex;
	}
	else
	{
		ImGui::Text("Parent: None");
	}

	ImGui::Text("Space:");
	ImGui::SameLine();
	if (ImGui::Button((_editSpace == World) ? "World" : "Local"))
		_editSpace = (_editSpace == World) ? Local : World;

	float inputWidth = 96.0f;
	bool isChanged = false;

	ImGuiInputTextFlags floatInputFlags = ImGuiInputTextFlags_EnterReturnsTrue;

	ImGui::PushID("Position");
	{
		XMFLOAT3A entPos = ent->GetTransform()->GetPosition(_editSpace);

		ImGui::Text("Position: ");
		ImGui::SameLine();
		if (ImGui::InputFloat3("", &entPos.x, "%.3f", floatInputFlags))
		{
			isChanged = true;
		}

		if (isChanged)
			ent->GetTransform()->SetPosition(entPos, _editSpace);
	}
	ImGui::PopID();

	ImGui::PushID("Rotation");
	{
		float degreeConversion = 180.0f / XM_PI;
		isChanged = false;
		XMFLOAT3A entRot = ent->GetTransform()->GetEuler(_editSpace);
		XMFLOAT3A entRotDeg = {
			entRot.x * degreeConversion,
			entRot.y * degreeConversion,
			entRot.z * degreeConversion
		};

		ImGui::Text("Rotation: ");
		ImGui::SameLine();
		if (ImGui::InputFloat3("", &entRotDeg.x, "%.3f", floatInputFlags))
		{
			isChanged = true;
		}

		if (isChanged)
		{
			entRot = {
				entRotDeg.x / degreeConversion,
				entRotDeg.y / degreeConversion,
				entRotDeg.z / degreeConversion
			};

			ent->GetTransform()->SetEuler(entRot, _editSpace);
		}
	}
	ImGui::PopID();

	ImGui::PushID("Scale");
	{
		isChanged = false;
		XMFLOAT3A entScale = ent->GetTransform()->GetScale(_editSpace);

		ImGui::Text("Scale:    ");
		ImGui::SameLine();
		if (ImGui::InputFloat3("", &entScale.x, "%.3f", floatInputFlags))
		{
			isChanged = true;
		}

		if (isChanged)
			ent->GetTransform()->SetScale(entScale, _editSpace);
	}
	ImGui::PopID();
		
	ImGui::PushID("Transform Catalogue");
	if (ImGui::CollapsingHeader("Transform Catalogue"))
	{
		ImGuiChildFlags childFlags = 0;
		childFlags |= ImGuiChildFlags_Border;
		childFlags |= ImGuiChildFlags_ResizeY;

		ImGui::BeginChild("Transform Catalogue", ImVec2(0, 150), childFlags);
		ImGui::Text("Note: Getters write the input. Setters read the input.");
		ImGui::Text("Methods with an additional float parameter use the w-value.");
		ImGui::Text("The reference space selected above is used for all methods.");

		static XMFLOAT4A parameter = { 0, 0, 0, 0 };
		ImGui::Text("Input: ");
		ImGui::SameLine();
		ImGui::InputFloat4("", &parameter.x, "%.3f", floatInputFlags);

		Transform *trans = ent->GetTransform();
		XMFLOAT4A *vec4 = &parameter;
		XMFLOAT3A *vec3 = reinterpret_cast<XMFLOAT3A*>(&parameter);
		float *vec1 = &parameter.x;
		
		if (ImGui::TreeNode("Getters"))
		{
			//ImGui::BeginChild("", ImVec2(0, 150), childFlags);
			if (ImGui::Button("Vec3 GetRight()"))
				std::memcpy(vec3, &trans->GetRight(_editSpace), sizeof(XMFLOAT3));

			if (ImGui::Button("Vec3 GetUp()"))
				std::memcpy(vec3, &trans->GetUp(_editSpace), sizeof(XMFLOAT3));

			if (ImGui::Button("Vec3 GetForward()"))
				std::memcpy(vec3, &trans->GetForward(_editSpace), sizeof(XMFLOAT3));
			
			if (ImGui::Button("Vec3 GetPosition()"))
				std::memcpy(vec3, &trans->GetPosition(_editSpace), sizeof(XMFLOAT3));
			
			if (ImGui::Button("Quat GetRotation()"))
				std::memcpy(vec4, &trans->GetRotation(_editSpace), sizeof(XMFLOAT4));

			if (ImGui::Button("Vec3 GetScale()"))
				std::memcpy(vec3, &trans->GetScale(_editSpace), sizeof(XMFLOAT3));

			if (ImGui::Button("Vec3 GetEuler()"))
				(*vec3) = trans->GetEuler(_editSpace), sizeof(XMFLOAT3);

			//ImGui::EndChild();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Setters"))
		{
			//ImGui::BeginChild("", ImVec2(0, 150), childFlags);

			if (ImGui::Button("SetPosition(Vec3)"))
				trans->SetPosition(*vec3, _editSpace);

			if (ImGui::Button("SetRotation(Quat)"))
				trans->SetRotation(*vec4, _editSpace);

			if (ImGui::Button("SetScale(Vec3)"))
				trans->SetScale(*vec3, _editSpace);

			if (ImGui::Button("Move(Vec3)"))
				trans->Move(*vec3, _editSpace);

			if (ImGui::Button("Rotate(Vec3)"))
				trans->Rotate(*vec3, _editSpace);

			if (ImGui::Button("Scale(Vec3)"))
				trans->Scale(*vec3, _editSpace);

			if (ImGui::Button("MoveRelative(Vec3)"))
				trans->MoveRelative(*vec3, _editSpace);

			if (ImGui::Button("RotateAxis(Vec3, float)"))
				trans->RotateAxis(*vec3, *vec1, _editSpace);

			if (ImGui::Button("SetEuler(Vec3)"))
				trans->SetEuler(*vec3, _editSpace);

			//ImGui::EndChild();
			ImGui::TreePop();
		}

		/*
		Maybe implement:
		SetMatrix(Mat mat) Optional
		Mat &GetLocalMatrix()
		Mat &GetWorldMatrix()
		*/
		ImGui::EndChild();
	}
	ImGui::PopID();

	return true;
}
bool Scene::RenderHierarchyUI(Entity* ent, UINT depth)
{
	if (depth > 0)
	{
		std::string indent = "";
		for (UINT i = 1; i < depth; i++)
			indent += "| ";
		indent += "|-";

		if (depth > 16)
		{
			ImGui::Text(std::format("{}[...]", indent).c_str());
			return true;
		}

		std::string entName = ent->GetName();

		if (_currSelection >= 0)
			if (ent == _sceneHolder.GetEntity(_currSelection))
				entName = std::format("<{}>", entName);

		UINT entIndex = _sceneHolder.GetEntityIndex(ent);

		ImGui::Text(indent.c_str());
		ImGui::SameLine();
		if (ImGui::SmallButton(std::format("{} ({})", entName, entIndex).c_str()))
			_currSelection = entIndex;

	}

	const std::vector<Entity*>* children = ent->GetChildren();
	for (auto& child : *children)
	{
		if (!child)
			continue;
		if (!RenderHierarchyUI(child, depth + 1))
			return false;
	}

	return true;
}

void Scene::DebugGenerateVolumeTreeStructure()
{
	static bool removePrevResult = false;
	if (removePrevResult)
	{
		int globalEntityCount = _globalEntities.size();
		for (int i = 0; i < globalEntityCount; i++)
		{
			Entity *ent = _globalEntities.at(i).get();

			if (ent->GetName() != "Culling Tree Wireframe")
				continue;

			_globalEntities.erase(_globalEntities.begin() + i);

			globalEntityCount--;
			i--;
		}

		removePrevResult = false;
		return;
	}

	removePrevResult = true;

	std::vector<BoundingBox> treeStructure;
	_sceneHolder.DebugGetTreeStructure(treeStructure);

	for (const BoundingBox &box : treeStructure)
	{
		static const UINT
			meshID = _content->GetMeshID("Mesh_WireframeCube"),
			textureID = _content->GetTextureID("Tex_Green"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = _content->GetTextureID("Tex_Green"),
			heightID = CONTENT_NULL;

		std::unique_ptr<Entity> entPtr = std::make_unique<Object>(-1, BoundingOrientedBox({ 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 }));
		Object *obj = reinterpret_cast<Object *>(entPtr.get());

		if (!obj->Initialize(_device, "Culling Tree Wireframe", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize culling tree wireframe object!");
			return;
		}

		const XMFLOAT4A
			center = { box.Center.x, box.Center.y, box.Center.z, 0.0f },
			scale = { box.Extents.x, box.Extents.y, box.Extents.z, 0.0f };

		entPtr->GetTransform()->SetPosition(center);
		entPtr->GetTransform()->SetScale(scale);

		_globalEntities.push_back(std::move(entPtr));
	}
}
void Scene::DebugGenerateEntityBounds()
{
	int entityCount = _sceneHolder.GetEntityCount();

	static bool removePrevResult = false;
	if (removePrevResult)
	{
		int globalEntityCount = _globalEntities.size();
		for (int i = 0; i < globalEntityCount; i++)
		{
			Entity *ent = _globalEntities.at(i).get();

			if (ent->GetName() != "Entity Bounds Wireframe")
				continue;

			_globalEntities.erase(_globalEntities.begin() + i);

			globalEntityCount--;
			i--;
		}

		removePrevResult = false;
		return;
	}

	removePrevResult = true;

	std::vector<BoundingOrientedBox> entityBounds;

	for (int i = 0; i < entityCount; i++)
	{
		Entity *ent = _sceneHolder.GetEntity(i);

		BoundingOrientedBox box;
		ent->StoreBounds(box);
		entityBounds.push_back(box);
	}

	for (const BoundingOrientedBox &box : entityBounds)
	{
		static const UINT
			meshID = _content->GetMeshID("Mesh_WireframeCube"),
			textureID = _content->GetTextureID("Tex_Blue"),
			normalID = CONTENT_NULL,
			specularID = CONTENT_NULL,
			reflectiveID = CONTENT_NULL,
			ambientID = _content->GetTextureID("Tex_Blue"),
			heightID = CONTENT_NULL;

		std::unique_ptr<Entity> entPtr = std::make_unique<Object>(-1, BoundingOrientedBox({ 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 }));
		Object *obj = reinterpret_cast<Object *>(entPtr.get());

		if (!obj->Initialize(_device, "Entity Bounds Wireframe", meshID, textureID, normalID, specularID, reflectiveID, ambientID, heightID))
		{
			ErrMsg("Failed to initialize entity bounds wireframe object!");
			return;
		}

		const XMFLOAT4A
			center = { box.Center.x, box.Center.y, box.Center.z, 0.0f },
			scale = { box.Extents.x, box.Extents.y, box.Extents.z, 0.0f };

		XMFLOAT4A boxOrientation = { box.Orientation.x, box.Orientation.y, box.Orientation.z, box.Orientation.w};

		entPtr->GetTransform()->SetPosition(center);
		entPtr->GetTransform()->SetRotation(boxOrientation);
		entPtr->GetTransform()->SetScale(scale);

		_globalEntities.push_back(std::move(entPtr));
	}
}
