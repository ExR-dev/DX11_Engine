#include "Cubemap.h"

#include "ErrMsg.h"


constexpr float UPDATE_INTERVAL = 0.0f;//0.025f;


Cubemap::Cubemap()
{
	_cameras.fill(nullptr);
	_rtvs.fill(nullptr);
	_uavs.fill(nullptr);
}

Cubemap::Cubemap(ID3D11Device *device, const UINT resolution, const float nearZ, const float farZ, const DirectX::XMFLOAT4A &initialPosition)
{
	if (!Initialize(device, resolution, nearZ, farZ, initialPosition))
		ErrMsg("Failed to initialize cubemap from constructor!");
}

Cubemap::~Cubemap()
{
	if (_dsView != nullptr)
		_dsView->Release();

	if (_dsTexture != nullptr)
		_dsTexture->Release();

	for (const auto &uav : _uavs)
		if (uav != nullptr)
			uav->Release();

	for (const auto &rtv : _rtvs)
		if (rtv != nullptr)
			rtv->Release();

	for (const CameraD3D11 *camera : _cameras)
		if (camera != nullptr)
			delete camera;
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

	_cameras[0]->LookX(DirectX::XM_PIDIV2);
	_cameras[0]->RotateRoll(DirectX::XM_PI);

	_cameras[1]->LookX(-DirectX::XM_PIDIV2);
	_cameras[1]->RotateRoll(DirectX::XM_PI);

	_cameras[2]->LookY(DirectX::XM_PIDIV2);

	_cameras[3]->LookY(-DirectX::XM_PIDIV2);

	_cameras[4]->LookX(DirectX::XM_PI);
	_cameras[4]->RotateRoll(DirectX::XM_PI);

	_cameras[5]->RotateRoll(DirectX::XM_PI);


	D3D11_TEXTURE2D_DESC textureDesc = { };
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = resolution;
	textureDesc.Height = resolution;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	if (!_texture.Initialize(device, nullptr, textureDesc, nullptr, false))
	{
		ErrMsg("Failed to initialize cubemap texture!");
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = 1;
	uavDesc.Texture2DArray.MipSlice = 0;
	
	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		if (FAILED(device->CreateRenderTargetView(_texture.GetTexture(), &rtvDesc, &_rtvs[i])))
		{
			ErrMsg(std::format("Failed to create cubemap rtv #{}!", i));
			return false;
		}

		uavDesc.Texture2DArray.FirstArraySlice = i;
		if (FAILED(device->CreateUnorderedAccessView(_texture.GetTexture(), &uavDesc, &_uavs[i])))
		{
			ErrMsg(std::format("Failed to create cubemap uav #{}!", i));
			return false;
		}
	}

	D3D11_TEXTURE2D_DESC depthTextureDesc = { };
	depthTextureDesc.Width = resolution;
	depthTextureDesc.Height = resolution;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.CPUAccessFlags = 0;
	depthTextureDesc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&depthTextureDesc, nullptr, &_dsTexture)))
	{
		ErrMsg("Failed to create cubemap depth stencil texture!");
		return false;
	}

	if (FAILED(device->CreateDepthStencilView(_dsTexture, nullptr, &_dsView)))
	{
		ErrMsg("Failed to create cubemap depth stencil view!");
		return false;
	}

	for (UINT i = 0; i < G_BUFFER_COUNT; i++)
	{
		if (!_gBuffers[i].Initialize(device, resolution, resolution, DXGI_FORMAT_R32G32B32A32_FLOAT, true))
		{
			ErrMsg(std::format("Failed to initialize cubemap g-buffer #{}!", i));
			return false;
		}
	}

	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;
	_viewport.Width = static_cast<float>(resolution);
	_viewport.Height = static_cast<float>(resolution);
	_viewport.MinDepth = 0;
	_viewport.MaxDepth = 1;

	return true;
}


bool Cubemap::Update(ID3D11DeviceContext *context, Time &time)
{
	_updateTimer += time.deltaTime;
	_doUpdate = (_updateTimer >= UPDATE_INTERVAL);
	if (_doUpdate) 
		_updateTimer = 0;

	if (!UpdateBuffers(context))
	{
		ErrMsg("Failed to update cubemap buffers!");
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


bool Cubemap::GetUpdate() const
{
	return _doUpdate;
}

CameraD3D11 *Cubemap::GetCamera(const UINT index) const
{
	return _cameras[index];
}

const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *Cubemap::GetGBuffers() const
{
	return &_gBuffers;
}

ID3D11RenderTargetView *Cubemap::GetRTV(const UINT index) const
{
	return _rtvs[index];
}

ID3D11UnorderedAccessView *Cubemap::GetUAV(const UINT index) const
{
	return _uavs[index];
}

ID3D11ShaderResourceView *Cubemap::GetSRV() const
{
	return _texture.GetSRV();
}

ID3D11DepthStencilView *Cubemap::GetDSV() const
{
	return _dsView;
}

const D3D11_VIEWPORT &Cubemap::GetViewport() const
{
	return _viewport;
}

void Cubemap::StoreBounds(DirectX::BoundingBox &bounds) const
{
	const ProjectionInfo projInfo = _cameras[0]->GetCurrProjectionInfo();
	const DirectX::XMFLOAT4A pos = _cameras[0]->GetPosition();

	bounds.Extents = { projInfo.farZ, projInfo.farZ, projInfo.farZ };
	bounds.Center = { pos.x, pos.y, pos.z };
}
