#include "InputLayoutD3D11.h"

#include "ErrMsg.h"


InputLayoutD3D11::~InputLayoutD3D11()
{
	if (_inputLayout != nullptr)
		_inputLayout->Release();
}


bool InputLayoutD3D11::AddInputElement(const Semantic &semantic)
{
	const UINT alignment = _elements.empty() ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;

	_semanticNames.push_back(semantic.name);
	_elements.push_back({
		semantic.name.c_str(),
		0,
		semantic.format,
		0,
		alignment,
		D3D11_INPUT_PER_VERTEX_DATA,
		0
	});

	return false;
}

bool InputLayoutD3D11::FinalizeInputLayout(ID3D11Device *device, const void *vsDataPtr, const size_t vsDataSize)
{
	if (FAILED(device->CreateInputLayout(
		_elements.data(), _elements.size(),
		vsDataPtr,
		vsDataSize,
		&_inputLayout)))
	{
		ErrMsg("Failed to finalize input layout!");
		return false;
	}
	return true;
}


ID3D11InputLayout *InputLayoutD3D11::GetInputLayout() const
{
	return _inputLayout;
}
