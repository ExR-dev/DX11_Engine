#pragma once

#include <d3d11_4.h>

class VertexBufferD3D11
{
private:
	ID3D11Buffer* _buffer = nullptr;
	UINT _nrOfVertices = 0;
	UINT _vertexSize = 0;

public:
	VertexBufferD3D11() = default;
	VertexBufferD3D11(ID3D11Device* device, UINT sizeOfVertex, 
		UINT nrOfVerticesInBuffer, const void* vertexData);
	~VertexBufferD3D11();
	VertexBufferD3D11(const VertexBufferD3D11& other) = delete;
	VertexBufferD3D11& operator=(const VertexBufferD3D11& other) = delete;
	VertexBufferD3D11(VertexBufferD3D11&& other) = delete;
	VertexBufferD3D11& operator=(VertexBufferD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, UINT sizeOfVertex,
		UINT nrOfVerticesInBuffer, const void* vertexData);

	UINT GetNrOfVertices() const;
	UINT GetVertexSize() const;
	ID3D11Buffer* GetBuffer() const;
};