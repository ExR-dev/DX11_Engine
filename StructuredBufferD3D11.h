#pragma once

#include <d3d11_4.h>


class StructuredBufferD3D11
{
private:
	ID3D11Buffer *_buffer = nullptr;
	ID3D11ShaderResourceView *_srv = nullptr;
	UINT _elementSize = 0;
	size_t _nrOfElements = 0;

public:
	StructuredBufferD3D11() = default;
	StructuredBufferD3D11(ID3D11Device *device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void *bufferData = nullptr, bool dynamic = true);
	~StructuredBufferD3D11();
	StructuredBufferD3D11(const StructuredBufferD3D11 &other) = delete;
	StructuredBufferD3D11 &operator=(const StructuredBufferD3D11 &other) = delete;
	StructuredBufferD3D11(StructuredBufferD3D11 &&other) = delete;
	StructuredBufferD3D11 operator=(StructuredBufferD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void *bufferData = nullptr, bool dynamic = true);

	[[nodiscard]] bool UpdateBuffer(ID3D11DeviceContext *context, const void *data) const;

	[[nodiscard]] UINT GetElementSize() const;
	[[nodiscard]] size_t GetNrOfElements() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSRV() const;
};