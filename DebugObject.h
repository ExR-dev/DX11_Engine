#pragma once

#include "ConstantBufferD3D11.h"
#include "PipelineHelper.h"
#include "Time.h"
#include "Content.h"
#include "Graphics.h"
#include "Transform.h"


class DebugObject
{
private:
	bool _initialized;
	Transform _transform;

	UINT _inputLayoutID = CONTENT_LOAD_ERROR;
	UINT _meshID = CONTENT_LOAD_ERROR;
	UINT _vsID = CONTENT_LOAD_ERROR;
	UINT _psID = CONTENT_LOAD_ERROR;
	UINT _texID = CONTENT_LOAD_ERROR;


public:
	DebugObject();
	~DebugObject();
	DebugObject(const DebugObject &other) = delete;
	DebugObject &operator=(const DebugObject &other) = delete;
	DebugObject(DebugObject &&other) = delete;
	DebugObject &operator=(DebugObject &&other) = delete;

	bool Initialize(ID3D11Device *device, UINT inputLayoutID, UINT meshID, UINT vsID, UINT psID, UINT texID);

	bool Update(ID3D11DeviceContext *context, const Time &time);
	bool Render(ID3D11DeviceContext *context, const Graphics &graphics, const Content &content);
};
