#include "Graphics.h"

#include <algorithm>

#include "ErrMsg.h"
#include "Entity.h"
#include "Emitter.h"
#include "D3D11Helper.h"
#include "Object.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"


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

	if (!_globalLightBuffer.Initialize(device, sizeof(float) * 4, &_ambientColor))
	{
		ErrMsg("Failed to initialize global light buffer!");
		return false;
	}

//#ifdef _DEBUG
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, immediateContext);
	ImGui::StyleColorsDark();
//#endif // _DEBUG

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


bool Graphics::BeginSceneRender()
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

bool Graphics::EndSceneRender(Time &time)
{
	if (!_isRendering)
	{
		ErrMsg("Failed to end rendering, rendering has not begun!");
		return false;
	}

	_ambientColor.w = time.time;
	if (!_globalLightBuffer.UpdateBuffer(_context, &_ambientColor))
	{
		ErrMsg("Failed to initialize global light buffer!");
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

	return true;
}


bool Graphics::RenderShadowCasters()
{
	if (_currSpotLightCollection == nullptr)
	{
		ErrMsg("Failed to render shadow casters, current spotlight collection is nullptr!");
		return false;
	}

	// Bind depth stage resources
	const UINT ilID = _content->GetInputLayoutID("IL_Fallback");
	if (_currInputLayoutID != ilID)
	{
		_context->IASetInputLayout(_content->GetInputLayout(ilID)->GetInputLayout());
		_currInputLayoutID = ilID;
	}

	const UINT vsID = _content->GetShaderID("VS_Depth");
	if (_currVsID != vsID)
	{
		if (!_content->GetShader(vsID)->BindShader(_context))
		{
			ErrMsg("Failed to bind depth-stage vertex shader!");
			return false;
		}
		_currVsID = vsID;
	}

	_context->PSSetShader(nullptr, nullptr, 0);

	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->RSSetState(_currSpotLightCollection->GetRasterizerState());
	_context->RSSetViewports(1, &_currSpotLightCollection->GetViewport());

	const MeshD3D11 *loadedMesh = nullptr;

	const UINT spotLightCount = _currSpotLightCollection->GetNrOfLights();
	for (UINT spotlight_i = 0; spotlight_i < spotLightCount; spotlight_i++)
	{
		ID3D11DepthStencilView *dsView = _currSpotLightCollection->GetShadowMapDSV(spotlight_i);
		_context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);

		// Skip rendering if disabled
		if (!_currSpotLightCollection->IsEnabled(spotlight_i))
			continue;

		_context->OMSetRenderTargets(0, nullptr, dsView);

		// Bind shadow-camera data
		const CameraD3D11 *spotlightCamera = _currSpotLightCollection->GetLightCamera(spotlight_i);

		if (!spotlightCamera->BindGeometryBuffers(_context))
		{
			ErrMsg(std::format("Failed to bind shadow-camera buffers for spotlight #{}!", spotlight_i));
			return false;
		}

		UINT entity_i = 0;
		for (const auto &[resources, instance] : spotlightCamera->GetRenderQueue())
		{
			if (static_cast<Entity *>(instance.subject)->GetType() != EntityType::OBJECT)
			{
				ErrMsg(std::format("Skipping depth-rendering for non-object #{}!", entity_i));
				return false;
			}

			// Bind shared entity data, skip data irrelevant for shadow mapping
			if (_currMeshID != resources.meshID)
			{
				loadedMesh = _content->GetMesh(resources.meshID);
				//if (!loadedMesh->BindMeshBuffers(_context, sizeof(float) * 4, 0)) // Only bind position
				// TODO: Check if stride works as expected
				if (!loadedMesh->BindMeshBuffers(_context))
				{ 
					ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", entity_i));
					return false;
				}
				_currMeshID = resources.meshID;
			}

			// Bind private entity data
			if (!static_cast<Object *>(instance.subject)->BindBuffers(_context))
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
	_context->RSSetState(nullptr); // TODO: Does this work?

	// Bind camera data
	if (!_currCamera->BindGeometryBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	// Bind geometry stage resources
	const UINT geometryInputLayoutID = _content->GetInputLayoutID("IL_Fallback");
	if (_currInputLayoutID != geometryInputLayoutID)
	{
		_context->IASetInputLayout(_content->GetInputLayout(geometryInputLayoutID)->GetInputLayout());
		_currInputLayoutID = geometryInputLayoutID;
	}

	const UINT vsID = _content->GetShaderID("VS_Geometry");
	if (_currVsID != vsID)
	{
		if (!_content->GetShader(vsID)->BindShader(_context))
		{
			ErrMsg("Failed to bind geometry vertex shader!");
			return false;
		}
		_currVsID = vsID;
	}

	const UINT psID = _content->GetShaderID("PS_Geometry");
	if (_currPsID != psID)
	{
		if (!_content->GetShader(psID)->BindShader(_context))
		{
			ErrMsg("Failed to bind geometry pixel shader!");
			return false;
		}
		_currPsID = psID;
	}

	const UINT ssID = _content->GetSamplerID("SS_Fallback");
	if (_currSamplerID != ssID)
	{
		ID3D11SamplerState *const ss = _content->GetSampler(ssID)->GetSamplerState();
		_context->PSSetSamplers(0, 1, &ss);
		_currSamplerID = ssID;
	}


	const MeshD3D11 *loadedMesh = nullptr;

	UINT entity_i = 0;
	for (const auto &[resources, instance] : _currCamera->GetRenderQueue())
	{
		if (static_cast<Entity *>(instance.subject)->GetType() != EntityType::OBJECT)
		{
			ErrMsg(std::format("Skipping depth-rendering for non-object #{}!", entity_i));
			return false;
		}

		// Bind shared entity data
		if (_currMeshID != resources.meshID)
		{
			loadedMesh = _content->GetMesh(resources.meshID);
			if (!loadedMesh->BindMeshBuffers(_context))
			{
				ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", entity_i));
				return false;
			}
			_currMeshID = resources.meshID;
		}
		else if (loadedMesh == nullptr)
			loadedMesh = _content->GetMesh(resources.meshID);

		if (_currTexID != resources.texID)
		{
			ID3D11ShaderResourceView *const srv = _content->GetTexture(resources.texID)->GetSRV();
			_context->PSSetShaderResources(0, 1, &srv);
			_currTexID = resources.texID;
		}

		// Bind private entity data
		if (!static_cast<Object *>(instance.subject)->BindBuffers(_context))
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
		for (size_t i = 0; i < subMeshCount; i++)
		{
			if (!loadedMesh->PerformSubMeshDrawCall(_context, i))
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, i));
				return false;
			}
		}

		entity_i++;
	}


	const std::vector<RenderInstance> emitters = _currCamera->GetEmitterQueue();
	const UINT emitterCount = emitters.size();
	if (emitterCount > 0)
	{
		// Bind particle emitter resources
		const UINT particleInputLayoutID = _content->GetInputLayoutID("IL_Null");
		if (_currInputLayoutID != particleInputLayoutID)
		{
			_context->IASetInputLayout(_content->GetInputLayout(particleInputLayoutID)->GetInputLayout());
			_currInputLayoutID = particleInputLayoutID;
		}

		const UINT particleVsID = _content->GetShaderID("VS_Particle");
		if (_currVsID != particleVsID)
		{
			if (!_content->GetShader(particleVsID)->BindShader(_context))
			{
				ErrMsg("Failed to bind particle vertex shader!");
				return false;
			}
			_currVsID = particleVsID;
		}

		if (!_content->GetShader(_content->GetShaderID("GS_Billboard"))->BindShader(_context))
		{
			ErrMsg("Failed to bind billboard geometry shader!");
			return false;
		}


		// Render particle emitters
		for (UINT i = 0; i < emitterCount; i++)
		{
			Emitter *emitter = static_cast<Emitter *>(emitters[i].subject);

			if (!emitter->BindBuffers(_context))
			{
				ErrMsg("Failed to bind emitter buffers!");
				return false;
			}

			if (!emitter->PerformDrawCall(_context))
			{
				ErrMsg("Failed to perform emitter draw call!");
				return false;
			}
		}

		// Unbind geometry shader
		_context->GSSetShader(nullptr, nullptr, 0);
	}

	// Unbind render targets
	for (auto &rtv : rtvs)
		rtv = nullptr;
	_context->OMSetRenderTargets(G_BUFFER_COUNT, rtvs, _dsView);

	return true;
}

bool Graphics::RenderLighting() const
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
	if (!_currSpotLightCollection->BindCSBuffers(_context))
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


bool Graphics::BeginUIRender() const
{
	_context->OMSetRenderTargets(1, &_rtv, _dsView);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Debug");

	return true;
}

bool Graphics::RenderUI(Time &time) const
{
	char fps[8]{};
	snprintf(fps, sizeof(fps), "%.2f", 1.0f / time.deltaTime);
	ImGui::Text(std::format("fps: {}", fps).c_str());

	ImGui::Text(std::format("Main Draws: {}", _currCamera->GetRenderQueue().size()).c_str());
	for (size_t i = 0; i < _currSpotLightCollection->GetNrOfLights(); i++)
	{
		const CameraD3D11 *spotlightCamera = _currSpotLightCollection->GetLightCamera(i);
		ImGui::Text(std::format("Spotlight #{} Draws: {}", i, spotlightCamera->GetRenderQueue().size()).c_str());
	}

	return true;
}

bool Graphics::EndUIRender() const
{
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
}


bool Graphics::EndFrame()
{
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

bool Graphics::ResetRenderState()
{
	_currCamera->ResetRenderQueue();

	for (size_t i = 0; i < _currSpotLightCollection->GetNrOfLights(); i++)
		_currSpotLightCollection->GetLightCamera(i)->ResetRenderQueue();

	_currInputLayoutID = CONTENT_LOAD_ERROR;
	_currMeshID = CONTENT_LOAD_ERROR;
	_currVsID = CONTENT_LOAD_ERROR;
	_currPsID = CONTENT_LOAD_ERROR;
	_currTexID = CONTENT_LOAD_ERROR;

	_isRendering = false;
	return true;
}
