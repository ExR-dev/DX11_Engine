#include "StructuredBufferD3D11.h"

#include "ErrMsg.h"


StructuredBufferD3D11::StructuredBufferD3D11(ID3D11Device *device, const UINT sizeOfElement, const size_t nrOfElementsInBuffer, 
	const bool hasSRV, const bool hasUAV, const bool dynamic, void *bufferData)
{
	if (!Initialize(device, sizeOfElement, nrOfElementsInBuffer, hasSRV, hasUAV, dynamic, bufferData))
		ErrMsg("Failed to initialize structured buffer from constructor!");
}

StructuredBufferD3D11::~StructuredBufferD3D11()
{
	if (_uav != nullptr)
		_uav->Release();

	if (_srv != nullptr)
		_srv->Release();

	if (_buffer != nullptr)
		_buffer->Release();
}

bool StructuredBufferD3D11::Initialize(ID3D11Device *device, const UINT sizeOfElement, const size_t nrOfElementsInBuffer,
	const bool hasSRV, const bool hasUAV, const bool dynamic, void *bufferData)
{
	_elementSize = sizeOfElement;
	_nrOfElements = nrOfElementsInBuffer;

	D3D11_BUFFER_DESC structuredBufferDesc;
	structuredBufferDesc.ByteWidth = _elementSize * static_cast<UINT>(_nrOfElements);
	structuredBufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	structuredBufferDesc.BindFlags = hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	structuredBufferDesc.BindFlags |= hasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0;
	structuredBufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	structuredBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	structuredBufferDesc.StructureByteStride = _elementSize;

	D3D11_SUBRESOURCE_DATA srData = { };
	const D3D11_SUBRESOURCE_DATA *srDataPtr = nullptr;

	if (bufferData != nullptr)
	{
		srData.pSysMem = bufferData;
		srData.SysMemPitch = 0;
		srData.SysMemSlicePitch = 0;

		srDataPtr = &srData;
	}

	if (FAILED(device->CreateBuffer(&structuredBufferDesc, srDataPtr, &_buffer)))
	{
		ErrMsg("Failed to create structured buffer!");
		return false;
	}

	if (hasSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = static_cast<UINT>(_nrOfElements);

		if (FAILED(device->CreateShaderResourceView(_buffer, &srvDesc, &_srv)))
		{
			ErrMsg("Failed to create srv for structured buffer!");
			return false;
		}
	}

	if (hasUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = static_cast<UINT>(_nrOfElements);
		uavDesc.Buffer.Flags = 0;

		if (FAILED(device->CreateUnorderedAccessView(_buffer, &uavDesc, &_uav)))
		{
			ErrMsg("Failed to create srv for structured buffer!");
			return false;
		}
	}

	return true;
}


bool StructuredBufferD3D11::UpdateBuffer(ID3D11DeviceContext *context, const void *data) const
{
	if (_buffer == nullptr)
	{
		ErrMsg("Structured buffer is not initialized!");
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	if (FAILED(context->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource)))
	{
		ErrMsg("Failed to update structured buffer!");
		return false;
	}

	memcpy(resource.pData, data, _elementSize * _nrOfElements);
	context->Unmap(_buffer, 0);
	return true;
}


UINT StructuredBufferD3D11::GetElementSize() const
{
	return _elementSize;
}

size_t StructuredBufferD3D11::GetNrOfElements() const
{
	return _nrOfElements;
}

ID3D11ShaderResourceView *StructuredBufferD3D11::GetSRV() const
{
	return _srv;
}

ID3D11UnorderedAccessView *StructuredBufferD3D11::GetUAV() const
{
	return _uav;
}
