#include "Graphics.h"

#include <algorithm>

#include "ErrMsg.h"
#include "Entity.h"
#include "D3D11Helper.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"


// private:

bool Graphics::FlushRenderQueue()
{
	// Bind camera data
	if (!_currCamera->BindGeometryBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	const MeshD3D11 *loadedMesh = nullptr;

	UINT i = 0;
	for (const auto &[resources, instance] : _renderInstances)
	{
		// Bind shared entity data
		if (_currInputLayoutID != resources.inputLayoutID)
		{
			_context->IASetInputLayout(_content->GetInputLayout(resources.inputLayoutID)->GetInputLayout());
			_currInputLayoutID = resources.inputLayoutID;
		}

		if (_currMeshID != resources.meshID)
		{
			loadedMesh = _content->GetMesh(resources.meshID);
			if (!loadedMesh->BindMeshBuffers(_context))
			{
				ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", i));
				return false;
			}
			_currMeshID = resources.meshID;
		}

		if (_currVsID != resources.vsID)
		{
			if (!_content->GetShader(resources.vsID)->BindShader(_context))
			{
				ErrMsg(std::format("Failed to bind vertex shader for instance #{}!", i));
				return false;
			}
			_currVsID = resources.vsID;
		}

		if (_currPsID != resources.psID)
		{
			if (!_content->GetShader(resources.psID)->BindShader(_context))
			{
				ErrMsg(std::format("Failed to bind pixel shader for instance #{}!", i));
				return false;
			}
			_currPsID = resources.psID;
		}

		if (_currTexID != resources.texID)
		{
			ID3D11ShaderResourceView *const srv = _content->GetTexture(resources.texID)->GetSRV();
			_context->PSSetShaderResources(0, 1, &srv);
			_currTexID = resources.texID;
		}

		if (_currSamplerID != resources.samplerID)
		{
			ID3D11SamplerState *const ss = _content->GetSampler(resources.samplerID)->GetSamplerState();
			_context->PSSetSamplers(0, 1, &ss);
			_currSamplerID = resources.samplerID;
		}

		// Bind private entity data
		if (!static_cast<Entity*>(instance.subject)->BindBuffers(_context))
		{
			ErrMsg(std::format("Failed to bind private buffers for instance #{}!", i));
			return false;
		}

		// Perform draw calls
		if (loadedMesh == nullptr)
		{
			ErrMsg(std::format("Failed to perform draw call for instance #{}, loadedMesh is nullptr!", i));
			return false;
		}

		const size_t subMeshCount = loadedMesh->GetNrOfSubMeshes();
		for (size_t j = 0; j < subMeshCount; j++)
		{
			if (!loadedMesh->PerformSubMeshDrawCall(_context, j))
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", i, j));
				return false;
			}
		}

		i++;
	}

	ID3D11RenderTargetView *rtvs[G_BUFFER_COUNT];
	for (i = 0; i < G_BUFFER_COUNT; i++)
		rtvs[i] = nullptr;
	_context->OMSetRenderTargets(G_BUFFER_COUNT, rtvs, _dsView);

	return true;
}

bool Graphics::ResetRenderState()
{
	_renderInstances.clear();

	_currInputLayoutID	= CONTENT_LOAD_ERROR;
	_currMeshID			= CONTENT_LOAD_ERROR;
	_currVsID			= CONTENT_LOAD_ERROR;
	_currPsID			= CONTENT_LOAD_ERROR;
	_currTexID			= CONTENT_LOAD_ERROR;

	_isRendering = false;
	return true;
}


// public:

Graphics::Graphics()
{
	_isSetup		= false;
	_isRendering	= false;

	_context	= nullptr;
	_content	= nullptr;

	_swapChain	= nullptr;
	_rtv		= nullptr;
	_dsTexture	= nullptr;
	_dsView		= nullptr;
	_uav		= nullptr;
	_viewport	= { };
}

Graphics::~Graphics()
{
	if (_uav != nullptr)
		_uav->Release();

	if (_dsView != nullptr)
		_dsView->Release();

	if (_dsTexture != nullptr)
		_dsTexture->Release();

	if (_rtv != nullptr)
		_rtv->Release();

	if (_swapChain != nullptr)
		_swapChain->Release();

	if (_isSetup)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}


bool Graphics::Setup(const UINT width, const UINT height, const HWND window, 
                     ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content)
{
	if (_isSetup)
	{
		ErrMsg("Failed to set up graphics, graphics has already been set up!");
		return false;
	}

	if (!SetupD3D11(width, height, window, device, immediateContext, 
			_swapChain, _rtv, _dsTexture, _dsView, _uav, _viewport))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}

	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
	{
		if (!_gBuffers[i].Initialize(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, true))
		{
			ErrMsg(std::format("Failed to initialize g-buffer #{}!", i));
			return false;
		}
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, immediateContext);
	ImGui::StyleColorsDark();

	_context = immediateContext;
	_content = content;

	_isSetup = true;
	return true;
}


ID3D11RenderTargetView *Graphics::GetRTV() const
{
	return _rtv;
}

ID3D11Texture2D *Graphics::GetDsTexture() const
{
	return _dsTexture;
}

ID3D11DepthStencilView *Graphics::GetDsView() const
{
	return _dsView;
}

D3D11_VIEWPORT &Graphics::GetViewport()
{
	return _viewport;
}


bool Graphics::SetCamera(CameraD3D11 *camera)
{
	if (camera == nullptr)
	{
		ErrMsg("Failed to set camera, camera is nullptr!");
		return false;
	}

	_currCamera = camera;
	return true;
}


bool Graphics::BeginRender()
{
	if (!_isSetup)
	{
		ErrMsg("Failed to begin rendering, graphics has not been set up!");
		return false;
	}

	if (_isRendering)
	{
		ErrMsg("Failed to begin rendering, rendering has already begun!");
		return false;
	}
	_isRendering = true;

	constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	ID3D11RenderTargetView *rtvs[G_BUFFER_COUNT];

	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
	{
		rtvs[i] = _gBuffers[i].GetRTV();
		_context->ClearRenderTargetView(rtvs[i], clearColor);
	}
	_context->OMSetRenderTargets(G_BUFFER_COUNT, rtvs, _dsView);

	_context->ClearDepthStencilView(_dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->RSSetViewports(1, &_viewport);

	return true;
}

bool Graphics::QueueRender(const ResourceGroup &resources, const RenderInstance &instance)
{
	if (!_isRendering)
	{
		ErrMsg("Failed to queue render object, rendering has not begun!");
		return false;
	}

	_renderInstances.insert({ resources, instance });
	return true;
}

bool Graphics::EndRender(const Time &time)
{
	if (!_isRendering)
	{
		ErrMsg("Failed to end rendering, rendering has not begun!");
		return false;
	}

	if (!FlushRenderQueue())
	{
		ErrMsg("Failed to flush render queue!");
		return false;
	}





	if (!_content->GetShader("LightingCShader")->BindShader(_context))
	{
		ErrMsg(std::format("Failed to bind compute shader!"));
		return false;
	}

	// Bind camera data
	if (!_currCamera->BindLightingBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	ID3D11ShaderResourceView *srvs[G_BUFFER_COUNT];
	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
		srvs[i] = _gBuffers[i].GetSRV();
	_context->CSSetShaderResources(0, 3, srvs);

	_context->CSSetUnorderedAccessViews(0, 1, &_uav, nullptr);

	_context->Dispatch(_viewport.Width / 8, _viewport.Height / 8, 1);






	// ImGui
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Debug");

	ImGui::Text(std::format("ms: {}", time.deltaTime).c_str());
	ImGui::Text(std::format("fps: {}", 1.0f / time.deltaTime).c_str());

	ImGui::Text(std::format("Objects: {}", _renderInstances.size()).c_str());

	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	if (FAILED(_swapChain->Present(1, 0)))
	{
		ErrMsg("Failed to present geometry!");
		return false;
	}

	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
		srvs[i] = nullptr;
	_context->CSSetShaderResources(0, 3, srvs);

	if (!ResetRenderState())
	{
		ErrMsg("Failed to reset render state!");
		return false;
	}

	return true;
}
