#include "ConstantBufferD3D11.h"

#include "ErrMsg.h"


ConstantBufferD3D11::ConstantBufferD3D11(ID3D11Device *device, const size_t byteSize, const void *initialData)
{
	Initialize(device, byteSize, initialData);
}

ConstantBufferD3D11::~ConstantBufferD3D11()
{
	if (_buffer != nullptr)
		_buffer->Release();
}

ConstantBufferD3D11::ConstantBufferD3D11(ConstantBufferD3D11 &&other) noexcept
{
	_buffer = other._buffer;
	_bufferSize = other._bufferSize;

	other._buffer = nullptr;
	other._bufferSize = 0;
}

ConstantBufferD3D11 &ConstantBufferD3D11::operator=(ConstantBufferD3D11 &&other) noexcept
{
	// TODO
	if (this != &other)
	{
		_buffer = other._buffer;
		_bufferSize = other._bufferSize;

		other._buffer = nullptr;
		other._bufferSize = 0;
	}

	return *this;
}


bool ConstantBufferD3D11::Initialize(ID3D11Device *device, const size_t byteSize, const void *initialData)
{
	_bufferSize = byteSize;

	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.ByteWidth = static_cast<UINT>(byteSize);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = (initialData == nullptr) ? new char[byteSize] : initialData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&bufferDesc, &srData, &_buffer)))
	{
		ErrMsg("Failed to create constant buffer!");

		if (initialData == nullptr)
			delete[] static_cast<const char*>(srData.pSysMem);
		return false;
	}

	if (initialData == nullptr)
		delete[] static_cast<const char*>(srData.pSysMem);
	return true;
}


size_t ConstantBufferD3D11::GetSize() const
{
	return _bufferSize;
}

ID3D11Buffer *ConstantBufferD3D11::GetBuffer() const
{
	return _buffer;
}


bool ConstantBufferD3D11::UpdateBuffer(ID3D11DeviceContext *context, const void *data) const
{
	D3D11_MAPPED_SUBRESOURCE resource;
	if (FAILED(context->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource)))
	{
		ErrMsg("Failed to update constant buffer!");
		return false;
	}

	memcpy(resource.pData, data, _bufferSize);
	context->Unmap(_buffer, 0);
	return true;
}
