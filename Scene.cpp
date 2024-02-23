#include "Scene.h"

#include "ErrMsg.h"


Scene::Scene()
{
	_initialized = false;

	for (UINT i = 0; i < 3; i++)
	{
		_entities.push_back(new Entity(i));
	}

	_camera = new CameraD3D11();
}

Scene::~Scene()
{
	const size_t entityCount = _entities.size();
	for (size_t i = 0; i < entityCount; i++)
		delete _entities.at(i);

	delete _camera;
}

bool Scene::Initialize(ID3D11Device *device)
{
	if (_initialized)
		return false;

	_device = device;

	if (!_camera->Initialize(device, { 75.0f, 16.0f/9.0f, 0.1f, 10.0f }, {0.0f, 0.0f, -2.0f, 0.0f}))
	{
		ErrMsg("Failed to initialize camera!");
		return false;
	}

	for (int i = 0; i < _entities.size(); i++)
	{
		Entity *ent = _entities.at(i);

		if (!ent->Initialize(device, 0, i, 0, 1, 0))
		{
			ErrMsg(std::format("Failed to initialize entity #{}!", i));
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

	if (input.GetKey(KeyCode::P) == KeyState::Pressed)
	{ // Create new entity
		_entities.push_back(new Entity(static_cast<UINT>(_entities.size())));
		Entity *ent = _entities.back();

		if (!ent->Initialize(_device, 0, 0, 0, 1, 0))
		{
			ErrMsg(std::format("Failed to initialize entity #{}!", _entities.size() - 1));
			return false;
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
	{ // Moving camera
		if (input.GetKey(KeyCode::D) == KeyState::Held)
			_camera->MoveRight(time.deltaTime * 2.0f);
		else if (input.GetKey(KeyCode::A) == KeyState::Held)
			_camera->MoveRight(-time.deltaTime * 2.0f);

		if (input.GetKey(KeyCode::Space) == KeyState::Held)
			_camera->MoveUp(time.deltaTime * 2.0f);
		else if (input.GetKey(KeyCode::X) == KeyState::Held)
			_camera->MoveUp(-time.deltaTime * 2.0f);

		if (input.GetKey(KeyCode::W) == KeyState::Held)
			_camera->MoveForward(time.deltaTime * 2.0f);
		else if (input.GetKey(KeyCode::S) == KeyState::Held)
			_camera->MoveForward(-time.deltaTime * 2.0f);

		const MouseState mState = input.GetMouse();
		_camera->LookX(static_cast<float>(mState.dx) / 360.0f);
		_camera->LookY(static_cast<float>(-mState.dy) / 360.0f);
	}
	else
	{ // Moving entity
		static bool isRotating = false;
		if (input.GetKey(KeyCode::R) == KeyState::Pressed)
			isRotating = !isRotating;

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
		else
			entityTransform->Move(transformationVector);
	}

	if (!_camera->UpdateBuffers(context))
	{
		ErrMsg("Failed to update camera buffers!");
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

bool Scene::Render(Graphics *graphics)
{
	if (!_initialized)
		return false;

	if (!graphics->SetCamera(_camera))
	{
		ErrMsg("Failed to set camera!");
		return false;
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