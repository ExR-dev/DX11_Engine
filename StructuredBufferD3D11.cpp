#include "StructuredBufferD3D11.h"

#include "ErrMsg.h"


StructuredBufferD3D11::StructuredBufferD3D11(ID3D11Device *device, const UINT sizeOfElement, const size_t nrOfElementsInBuffer, void *bufferData, const bool dynamic)
{
	if (!Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic))
		ErrMsg("Failed to initialize structured buffer in constructor!");
}

StructuredBufferD3D11::~StructuredBufferD3D11()
{
	if (_srv != nullptr)
		_srv->Release();

	if (_buffer != nullptr)
		_buffer->Release();
}

bool StructuredBufferD3D11::Initialize(ID3D11Device *device, const UINT sizeOfElement, const size_t nrOfElementsInBuffer, void *bufferData, const bool dynamic)
{
	_elementSize = sizeOfElement;
	_nrOfElements = nrOfElementsInBuffer;

	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.ByteWidth = _elementSize * static_cast<UINT>(_nrOfElements);
	bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = _elementSize;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = (bufferData == nullptr) ? new char[bufferDesc.ByteWidth] : bufferData;
	srData.pSysMem = bufferData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&bufferDesc, &srData, &_buffer)))
	{
		ErrMsg("Failed to create structured buffer!");

		if (bufferData == nullptr)
			delete[] static_cast<const char *>(srData.pSysMem);
		return false;
	}

	if (bufferData == nullptr)
		delete[] static_cast<const char *>(srData.pSysMem);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = static_cast<UINT>(_nrOfElements);

	if (FAILED(device->CreateShaderResourceView(_buffer, &srvDesc, &_srv)))
	{
		ErrMsg("Failed to create shader resource view!");
		return false;
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
