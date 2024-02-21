#include "Scene.h"

#include "ErrMsg.h"


Scene::Scene()
{
	_initialized = false;

	for (int i = 0; i < 1; i++)
	{
		_entities.push_back(new Entity());
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

	if (!_camera->Initialize(device, { 60.0f, 1.0f, 0.1f, 10.0f }, {0.0f, 0.0f, -2.0f, 0.0f}))
	{
		ErrMsg("Failed to initialize camera!");
		return false;
	}

	for (int i = 0; i < _entities.size(); i++)
	{
		Entity *ent = _entities.at(i);

		if (!ent->Initialize(device, 0, 0, 0, 1, 0))
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


	if (input.GetKey(KeyCode::D))
		_camera->MoveRight(time.deltaTime * 2.0f);
	else if (input.GetKey(KeyCode::A))
		_camera->MoveRight(-time.deltaTime * 2.0f);

	if (input.GetKey(KeyCode::Space))
		_camera->MoveUp(time.deltaTime * 2.0f);
	else if (input.GetKey(KeyCode::X))
		_camera->MoveUp(-time.deltaTime * 2.0f);

	if (input.GetKey(KeyCode::W))
		_camera->MoveForward(time.deltaTime * 2.0f);
	else if (input.GetKey(KeyCode::S))
		_camera->MoveForward(-time.deltaTime * 2.0f);


	if (input.GetKey(KeyCode::Right))
		_camera->LookX(time.deltaTime);
	else if (input.GetKey(KeyCode::Left))
		_camera->LookX(-time.deltaTime);

	if (input.GetKey(KeyCode::Up))
		_camera->LookY(time.deltaTime);
	else if (input.GetKey(KeyCode::Down))
		_camera->LookY(-time.deltaTime);


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

	graphics->SetCamera(_camera);

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