#include "VertexBufferD3D11.h"


VertexBufferD3D11::VertexBufferD3D11(ID3D11Device *device, const UINT sizeOfVertex, const UINT nrOfVerticesInBuffer, const void *vertexData)
{
	_vertexSize = sizeOfVertex;
	_nrOfVertices = nrOfVerticesInBuffer;

	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.ByteWidth = sizeof(vertexData);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = vertexData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &srData, &_buffer);
}

VertexBufferD3D11::~VertexBufferD3D11()
{
	_buffer->Release();
}


void VertexBufferD3D11::Initialize(ID3D11Device *device, const UINT sizeOfVertex, const UINT nrOfVerticesInBuffer, const void *vertexData)
{
	if (_buffer != nullptr)
		_buffer->Release();

	_vertexSize = sizeOfVertex;
	_nrOfVertices = nrOfVerticesInBuffer;

	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.ByteWidth = sizeof(vertexData);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = vertexData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &srData, &_buffer);
}

UINT VertexBufferD3D11::GetNrOfVertices() const
{
	return _nrOfVertices;
}

UINT VertexBufferD3D11::GetVertexSize() const
{
	return _vertexSize;
}

ID3D11Buffer *VertexBufferD3D11::GetBuffer() const
{
	return _buffer;
}