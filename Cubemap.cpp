#include "Cubemap.h"

#include "ErrMsg.h"


Cubemap::Cubemap()
{
	_cameras.fill(nullptr);
}

Cubemap::Cubemap(ID3D11Device *device, const float nearZ, const float farZ, const XMFLOAT4A &initialPosition)
{
	if (!Initialize(device, nearZ, farZ, initialPosition))
		ErrMsg("Failed to setup cubemap cameras in constructor!");
}

Cubemap::~Cubemap()
{
	for (const CameraD3D11 *camera : _cameras)
	{
		if (camera != nullptr)
			delete camera;
	}
}

bool Cubemap::Initialize(ID3D11Device *device, const float nearZ, const float farZ, const XMFLOAT4A &initialPosition)
{
	const ProjectionInfo projInfo {
		90.0f,
		1.0f,
		nearZ,
		farZ
	};

	for (size_t i = 0; i < 6; i++)
	{
		if (_cameras[i] != nullptr)
		{
			ErrMsg(std::format("Cubemap camera #{} is not nullptr!", i));
			return false;
		}

		_cameras[i] = new CameraD3D11();
		if (!_cameras[i]->Initialize(device, projInfo, initialPosition))
		{
			ErrMsg(std::format("Failed to initialize cubemap camera #{}!", i));
			return false;
		}
	}

	_cameras[1]->LookX(90.0f * XM_PI / 180.0f);
	_cameras[2]->LookX(180.0f * XM_PI / 180.0f);
	_cameras[3]->LookX(270.0f * XM_PI / 180.0f);
	_cameras[4]->LookY(-90.0f * XM_PI / 180.0f);
	_cameras[5]->LookY(90.0f * XM_PI / 180.0f);

	return true;
}


bool Cubemap::UpdateBuffers(ID3D11DeviceContext *context) const
{
	for (CameraD3D11 *camera : _cameras)
	{
		if (!camera->UpdateBuffers(context))
		{
			ErrMsg("Failed to update cubemap camera buffers!");
			return false;
		}
	}

	return true;
}


CameraD3D11 *Cubemap::GetCamera(UINT index) const
{
	if (index >= _cameras.size())
	{
		ErrMsg(std::format("Failed to get camera, index ({}) out of bounds!", index));
		return nullptr;
	}

	return _cameras[index];
}
