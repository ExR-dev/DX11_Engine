#include "Graphics.h"

#include <algorithm>

#include "ErrMsg.h"
#include "Entity.h"
#include "D3D11Helper.h"

#ifdef _DEBUG
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#endif // _DEBUG


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

#ifdef _DEBUG
	if (_isSetup)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
#endif // _DEBUG
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

	constexpr XMFLOAT4A ambientColor = { 1.0f,  1.0f,  1.0f,  0.10f };
	if (!_globalLightBuffer.Initialize(device, sizeof(float) * 4, &ambientColor))
	{
		ErrMsg("Failed to initialize global light buffer!");
		return false;
	}

#ifdef _DEBUG
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, immediateContext);
	ImGui::StyleColorsDark();
#endif // _DEBUG

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

bool Graphics::SetSpotlightCollection(SpotLightCollectionD3D11* spotlights)
{
	if (spotlights == nullptr)
	{
		ErrMsg("Failed to set spot light collection, collection is nullptr!");
		return false;
	}

	_currSpotLightCollection = spotlights;
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

	if (!RenderShadowCasters())
	{
		ErrMsg("Failed to render shadow casters!");
		return false;
	}

	if (!RenderGeometry())
	{
		ErrMsg("Failed to render geometry!");
		return false;
	}

	if (!RenderLighting())
	{
		ErrMsg("Failed to render lighting!");
		return false;
	}


#ifdef _DEBUG
	// ImGui
	_context->OMSetRenderTargets(1, &_rtv, _dsView);

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
#endif // _DEBUG

	if (FAILED(_swapChain->Present(1, 0)))
	{
		ErrMsg("Failed to present geometry!");
		return false;
	}

	if (!ResetRenderState())
	{
		ErrMsg("Failed to reset render state!");
		return false;
	}

	return true;
}



bool Graphics::RenderShadowCasters()
{
	if (_currSpotLightCollection == nullptr)
	{
		ErrMsg("Failed to render shadow casters, current spotlight collection is nullptr!");
		return false;
	}

	const UINT depthVShaderID = _content->GetShaderID("VS_Depth");
	if (_currVsID != depthVShaderID)
		if (!_content->GetShader(depthVShaderID)->BindShader(_context))
		{
			ErrMsg("Failed to bind depth vertex shader!");
			return false;
		}
	_currVsID = depthVShaderID;
	_context->PSSetShader(nullptr, nullptr, 0);

	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->RSSetViewports(1, &_currSpotLightCollection->GetViewport());

	const MeshD3D11 *loadedMesh = nullptr;

	const UINT spotLightCount = _currSpotLightCollection->GetNrOfLights();
	for (UINT spotlight_i = 0; spotlight_i < spotLightCount; spotlight_i++)
	{
		ID3D11DepthStencilView *dsView = _currSpotLightCollection->GetShadowMapDSV(spotlight_i);
		_context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);
		_context->OMSetRenderTargets(0, nullptr, dsView);

		// Bind shadow-camera data
		if (!_currSpotLightCollection->GetLightCamera(spotlight_i)->BindGeometryBuffers(_context))
		{
			ErrMsg(std::format("Failed to bind shadow-camera buffers for spotlight #{}!", spotlight_i));
			return false;
		}

		UINT entity_i = 0;
		for (const auto &[resources, instance] : _renderInstances)
		{
			// Bind shared entity data, skip data irrelevant for shadow mapping
			if (_currInputLayoutID != resources.inputLayoutID)
			{
				_context->IASetInputLayout(_content->GetInputLayout(resources.inputLayoutID)->GetInputLayout());
				_currInputLayoutID = resources.inputLayoutID;
			}

			if (_currMeshID != resources.meshID)
			{
				loadedMesh = _content->GetMesh(resources.meshID);
				//if (!loadedMesh->BindMeshBuffers(_context, sizeof(float) * 4, 0)) // Only bind position
				// TODO: Verify that stride works as expected
				if (!loadedMesh->BindMeshBuffers(_context))
				{ 
					ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", entity_i));
					return false;
				}
				_currMeshID = resources.meshID;
			}

			// Bind private entity data
			if (!static_cast<Entity *>(instance.subject)->BindBuffers(_context))
			{
				ErrMsg(std::format("Failed to bind private buffers for instance #{}!", entity_i));
				return false;
			}

			// Perform draw calls
			if (loadedMesh == nullptr)
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, loadedMesh is nullptr!", entity_i));
				return false;
			}

			const size_t subMeshCount = loadedMesh->GetNrOfSubMeshes();
			for (size_t submesh_i = 0; submesh_i < subMeshCount; submesh_i++)
			{
				if (!loadedMesh->PerformSubMeshDrawCall(_context, submesh_i))
				{
					ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, submesh_i));
					return false;
				}
			}

			entity_i++;
		}
	}

	// Unbind render target
	_context->OMSetRenderTargets(0, nullptr, nullptr);

	return true;
}

bool Graphics::RenderGeometry()
{
	constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Clear & bind render targets
	ID3D11RenderTargetView *rtvs[G_BUFFER_COUNT] = { };
	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
	{
		rtvs[i] = _gBuffers[i].GetRTV();
		_context->ClearRenderTargetView(rtvs[i], clearColor);
	}
	_context->OMSetRenderTargets(G_BUFFER_COUNT, rtvs, _dsView);

	_context->ClearDepthStencilView(_dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->RSSetViewports(1, &_viewport);

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
		else if (loadedMesh == nullptr)
			loadedMesh = _content->GetMesh(resources.meshID);

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
		if (!static_cast<Entity *>(instance.subject)->BindBuffers(_context))
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

	// Unbind render targets
	for (auto &rtv : rtvs)
		rtv = nullptr;
	_context->OMSetRenderTargets(G_BUFFER_COUNT, rtvs, _dsView);

	return true;
}

bool Graphics::RenderLighting()
{
	if (!_content->GetShader("CS_Lighting")->BindShader(_context))
	{
		ErrMsg(std::format("Failed to bind compute shader!"));
		return false;
	}

	_context->CSSetUnorderedAccessViews(0, 1, &_uav, nullptr);

	// Bind compute shader resources
	ID3D11ShaderResourceView *srvs[G_BUFFER_COUNT] = { };
	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
		srvs[i] = _gBuffers[i].GetSRV();
	_context->CSSetShaderResources(0, 3, srvs);

	// Bind global light data
	ID3D11Buffer *const globalLightBuffer = _globalLightBuffer.GetBuffer();
	_context->CSSetConstantBuffers(0, 1, &globalLightBuffer);

	// Bind spotlight collection
	if (!_currSpotLightCollection->BindBuffers(_context))
	{
		ErrMsg("Failed to bind spotlight buffers!");
		return false;
	}

	ID3D11SamplerState *const ss = _content->GetSampler(0)->GetSamplerState();
	_context->CSSetSamplers(0, 1, &ss);

	// Bind camera lighting data
	if (!_currCamera->BindLightingBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		//return false;
	}

	// Send execution command
	_context->Dispatch(static_cast<UINT>(_viewport.Width / 8), static_cast<UINT>(_viewport.Height / 8), 1);

	// Unbind compute shader resources
	memset(srvs, 0, sizeof(srvs));
	_context->CSSetShaderResources(0, 3, srvs);

	return true;
}


bool Graphics::ResetRenderState()
{
	_renderInstances.clear();

	_currInputLayoutID = CONTENT_LOAD_ERROR;
	_currMeshID = CONTENT_LOAD_ERROR;
	_currVsID = CONTENT_LOAD_ERROR;
	_currPsID = CONTENT_LOAD_ERROR;
	_currTexID = CONTENT_LOAD_ERROR;

	_isRendering = false;
	return true;
}
