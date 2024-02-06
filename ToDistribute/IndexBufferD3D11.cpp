#include "IndexBufferD3D11.h"


IndexBufferD3D11::IndexBufferD3D11(ID3D11Device *device, const size_t nrOfIndicesInBuffer, const uint32_t *indexData)
{
	_nrOfIndices = nrOfIndicesInBuffer;

	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.ByteWidth = sizeof(indexData);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = indexData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &srData, &_buffer);
}

IndexBufferD3D11::~IndexBufferD3D11()
{
	_buffer->Release();
}


void IndexBufferD3D11::Initialize(ID3D11Device *device, const size_t nrOfIndicesInBuffer, const uint32_t *indexData)
{
	if (_buffer != nullptr)
		_buffer->Release();

	_nrOfIndices = nrOfIndicesInBuffer;

	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.ByteWidth = sizeof(indexData);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = indexData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &srData, &_buffer);
}

size_t IndexBufferD3D11::GetNrOfIndices() const
{
	return _nrOfIndices;
}

ID3D11Buffer *IndexBufferD3D11::GetBuffer() const
{
	return _buffer;
}