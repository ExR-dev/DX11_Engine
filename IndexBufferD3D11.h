#pragma once

#include <cstdint>

#include <d3d11_4.h>

class IndexBufferD3D11
{
private:
	ID3D11Buffer *_buffer = nullptr;
	size_t _nrOfIndices = 0;

public:
	IndexBufferD3D11() = default;
	IndexBufferD3D11(ID3D11Device *device, size_t nrOfIndicesInBuffer, const uint32_t *indexData);
	~IndexBufferD3D11();
	IndexBufferD3D11(const IndexBufferD3D11 &other) = delete;
	IndexBufferD3D11 &operator=(const IndexBufferD3D11 &other) = delete;
	IndexBufferD3D11(IndexBufferD3D11 &&other) = delete;
	IndexBufferD3D11 &operator=(IndexBufferD3D11 &&other) = delete;

	bool Initialize(ID3D11Device *device, size_t nrOfIndicesInBuffer, const uint32_t *indexData);

	[[nodiscard]] size_t GetNrOfIndices() const;
	[[nodiscard]] ID3D11Buffer *GetBuffer() const;
};