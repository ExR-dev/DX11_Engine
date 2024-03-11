#include "Cubemap.h"

#include "ErrMsg.h"


Cubemap::Cubemap()
{
	_cameras.fill(nullptr);
}

Cubemap::Cubemap(ID3D11Device *device, const UINT resolution, const float nearZ, const float farZ, const DirectX::XMFLOAT4A &initialPosition)
{
	if (!Initialize(device, resolution, nearZ, farZ, initialPosition))
		ErrMsg("Failed to setup cubemap cameras from constructor!");
}

Cubemap::~Cubemap()
{
	for (const CameraD3D11 *camera : _cameras)
	{
		if (camera != nullptr)
			delete camera;
	}
}

bool Cubemap::Initialize(ID3D11Device *device, const UINT resolution, const float nearZ, const float farZ, const DirectX::XMFLOAT4A &initialPosition)
{
	const ProjectionInfo projInfo {
		DirectX::XM_PIDIV2,
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

	_cameras[1]->LookX(DirectX::XM_PIDIV2 * 1);
	_cameras[2]->LookX(DirectX::XM_PIDIV2 * 2);
	_cameras[3]->LookX(DirectX::XM_PIDIV2 * 3);
	_cameras[4]->LookY(DirectX::XM_PIDIV2);
	_cameras[5]->LookY(-DirectX::XM_PIDIV2);


	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = resolution;
	textureDesc.Height = resolution;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	if (!_texture.Initialize(device, textureDesc, nullptr))
	{
		ErrMsg("Failed to initialize cubemap texture!");
		return false;
	}

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
