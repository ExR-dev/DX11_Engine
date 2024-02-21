#include "SamplerD3D11.h"

#include "ErrMsg.h"


SamplerD3D11::SamplerD3D11(ID3D11Device *device, const D3D11_TEXTURE_ADDRESS_MODE adressMode, const std::optional<std::array<float, 4>> &borderColour)
{
	if (!Initialize(device, adressMode, borderColour))
		ErrMsg("Failed to initialize sampler in constructor!");
}

SamplerD3D11::~SamplerD3D11()
{
	if (_sampler != nullptr)
		_sampler->Release();
}

bool SamplerD3D11::Initialize(ID3D11Device *device, const D3D11_TEXTURE_ADDRESS_MODE adressMode, const std::optional<std::array<float, 4>>&borderColors)
{
	D3D11_SAMPLER_DESC samplerDesc = { };
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = adressMode;
	samplerDesc.AddressV = adressMode;
	samplerDesc.AddressW = adressMode;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	if (borderColors.has_value())
	{
		samplerDesc.BorderColor[0] = borderColors.value()[0];
		samplerDesc.BorderColor[1] = borderColors.value()[1];
		samplerDesc.BorderColor[2] = borderColors.value()[2];
		samplerDesc.BorderColor[3] = borderColors.value()[3];
	}

	if (FAILED(device->CreateSamplerState(&samplerDesc, &_sampler)))
	{
		ErrMsg("Failed to create sampler state!");
		return false;
	}

	return true;
}


ID3D11SamplerState *SamplerD3D11::GetSamplerState() const
{
	return _sampler;
}
