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
	if (_tdss != nullptr)
		_tdss->Release();

	if (_tbs != nullptr)
		_tbs->Release();

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

// #ifdef _DEBUG
	if (_isSetup)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
// #endif // _DEBUG
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
			_swapChain, _rtv, _dsTexture, _dsView, _uav, _tbs, _tdss, _viewport))
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

bool Graphics::GetUpdateCubemap() const
{
	return _updateCubemap;
}


bool Graphics::SetCameras(CameraD3D11 *mainCamera, CameraD3D11 *viewCamera)
{
	if (mainCamera == nullptr)
	{
		ErrMsg("Failed to set camera, camera is nullptr!");
		return false;
	}
	
	if (viewCamera == nullptr)
		viewCamera = mainCamera;

	_currMainCamera = mainCamera;
	_currViewCamera = viewCamera;
	return true;
}

bool Graphics::SetCubemap(Cubemap *cubemap)
{
	if (cubemap == nullptr)
	{
		ErrMsg("Failed to set cubemap, cubemap is nullptr!");
		return false;
	}

	_currCubemap = cubemap;
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
		ErrMsg("Failed to update global light buffer!");
		return false;
	}

	if (!RenderShadowCasters())
	{
		ErrMsg("Failed to render shadow casters!");
		return false;
	}

	// Render cubemap cameras to cubemap view
	if (_updateCubemap && _currCubemap != nullptr)
		if (_currCubemap->GetUpdate())
		{
			CameraD3D11
				*mainCamera = _currMainCamera,
				*viewCamera = _currViewCamera;

			for (UINT i = 0; i < 6; i++)
			{
				_currMainCamera = _currCubemap->GetCamera(i);
				_currViewCamera = _currMainCamera;

				if (!RenderToTarget(
						_currCubemap->GetGBuffers(), 
						_currCubemap->GetRTV(i), 
						_currCubemap->GetUAV(i), 
						_currCubemap->GetDSV(), 
						&_currCubemap->GetViewport(), 
						false, 
						true))
				{
					ErrMsg(std::format("Failed to render to cubemap view #{}!", i));
					return false;
				}
			}

			_currMainCamera = mainCamera;
			_currViewCamera = viewCamera;
		}

	// Render main camera to screen view
	_renderOutput %= G_BUFFER_COUNT + 1;
	if (!RenderToTarget(nullptr, nullptr, nullptr, nullptr, nullptr, (_renderOutput != 0), false))
	{
		ErrMsg("Failed to render to screen view!");
		return false;
	}

	return true;
}


bool Graphics::RenderToTarget(
	const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *targetGBuffers,
	ID3D11RenderTargetView *targetRTV, 
	ID3D11UnorderedAccessView *targetUAV, 
	ID3D11DepthStencilView *targetDSV, 
	const D3D11_VIEWPORT *targetViewport, 
	const bool renderGBuffer,
	const bool cubemapStage)
{
	if (targetGBuffers == nullptr)	targetGBuffers = &_gBuffers;
	if (targetRTV == nullptr)		targetRTV = _rtv;
	if (targetUAV == nullptr)		targetUAV = _uav;
	if (targetDSV == nullptr)		targetDSV = _dsView;
	if (targetViewport == nullptr)	targetViewport = &_viewport;

	if (!RenderGeometry(targetGBuffers, targetDSV, targetViewport))
	{
		ErrMsg("Failed to render geometry!");
		return false;
	}

	if (!renderGBuffer)
	{
		if (!RenderLighting(targetGBuffers, targetUAV, targetViewport, cubemapStage))
		{
			ErrMsg("Failed to render lighting!");
			return false;
		}

		if (!RenderTransparency(targetRTV, targetDSV, targetViewport))
		{
			ErrMsg("Failed to render transparency!");
			return false;
		}
	}
	else
	{
		if (!RenderGBuffer(_renderOutput - 1))
		{
			ErrMsg(std::format("Failed to render g-buffer #{}!", _renderOutput - 1));
			return false;
		}
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
		for (const auto &[resources, instance] : spotlightCamera->GetGeometryQueue())
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

bool Graphics::RenderGeometry(const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *targetGBuffers, 
	ID3D11DepthStencilView *targetDSV, const D3D11_VIEWPORT *targetViewport)
{
	constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear & bind render targets
	ID3D11RenderTargetView *rtvs[G_BUFFER_COUNT] = { };
	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
	{
		rtvs[i] = targetGBuffers->at(i).GetRTV();
		_context->ClearRenderTargetView(rtvs[i], clearColor);
	}
	_context->OMSetRenderTargets(G_BUFFER_COUNT, rtvs, targetDSV);

	_context->ClearDepthStencilView(targetDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->RSSetViewports(1, targetViewport);
	_context->RSSetState(nullptr); // TODO: Does this work?

	// Bind camera data
	if (!_currViewCamera->BindGeometryBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	// Bind geometry stage resources
	static UINT geometryInputLayoutID = _content->GetInputLayoutID("IL_Fallback");
	if (_currInputLayoutID != geometryInputLayoutID)
	{
		_context->IASetInputLayout(_content->GetInputLayout(geometryInputLayoutID)->GetInputLayout());
		_currInputLayoutID = geometryInputLayoutID;
	}

	static UINT vsID = _content->GetShaderID("VS_Geometry");
	if (_currVsID != vsID)
	{
		if (!_content->GetShader(vsID)->BindShader(_context))
		{
			ErrMsg("Failed to bind geometry vertex shader!");
			return false;
		}
		_currVsID = vsID;
	}

	static UINT psID = _content->GetShaderID("PS_Geometry");
	if (_currPsID != psID)
	{
		if (!_content->GetShader(psID)->BindShader(_context))
		{
			ErrMsg("Failed to bind geometry pixel shader!");
			return false;
		}
		_currPsID = psID;
	}

	static UINT ssID = _content->GetSamplerID("SS_Fallback");
	if (_currSamplerID != ssID)
	{
		ID3D11SamplerState *const ss = _content->GetSampler(ssID)->GetSamplerState();
		_context->PSSetSamplers(0, 1, &ss);
		_currSamplerID = ssID;
	}

	static UINT defaultNormalID = _content->GetTextureMapID("TexMap_Default_Normal");
	if (_currNormalID != defaultNormalID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultNormalID)->GetSRV();
		_context->PSSetShaderResources(1, 1, &srv);
		_currNormalID = defaultNormalID;
	}

	static UINT defaultSpecularID = _content->GetTextureMapID("TexMap_Default_Specular");
	if (_currSpecularID != defaultSpecularID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultSpecularID)->GetSRV();
		_context->PSSetShaderResources(2, 1, &srv);
		_currSpecularID = defaultSpecularID;
	}

	static UINT defaultReflectiveID = _content->GetTextureMapID("TexMap_Default_Reflective");
	if (_currReflectiveID != defaultReflectiveID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultReflectiveID)->GetSRV();
		_context->PSSetShaderResources(3, 1, &srv);
		_currReflectiveID = defaultReflectiveID;
	}

	const MeshD3D11 *loadedMesh = nullptr;
	UINT entity_i = 0;
	for (const auto &[resources, instance] : _currMainCamera->GetGeometryQueue())
	{
		if (static_cast<Entity *>(instance.subject)->GetType() != EntityType::OBJECT)
		{
			ErrMsg(std::format("Skipping depth-rendering for non-object #{}!", entity_i));
			return false;
		}

		// Bind shared geometry resources
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

		if (resources.normalID != CONTENT_LOAD_ERROR)
			if (_currNormalID != resources.normalID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.normalID)->GetSRV();
				_context->PSSetShaderResources(1, 1, &srv);
				_currNormalID = resources.normalID;
			}

		if (resources.specularID != CONTENT_LOAD_ERROR)
			if (_currSpecularID != resources.specularID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.specularID)->GetSRV();
				_context->PSSetShaderResources(2, 1, &srv);
				_currSpecularID = resources.specularID;
			}

		if (resources.reflectiveID != CONTENT_LOAD_ERROR)
			if (_currSpecularID != resources.specularID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.reflectiveID)->GetSRV();
				_context->PSSetShaderResources(3, 1, &srv);
				_currReflectiveID = resources.reflectiveID;
			}

		// Bind private entity resources
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

	// Unbind render targets
	for (auto &rtv : rtvs)
		rtv = nullptr;
	_context->OMSetRenderTargets(G_BUFFER_COUNT, rtvs, nullptr);

	return true;
}

bool Graphics::RenderLighting(const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *targetGBuffers,
	ID3D11UnorderedAccessView *targetUAV, const D3D11_VIEWPORT *targetViewport, const bool useCubemapShader) const
{
	const std::string shaderName = useCubemapShader ? "CS_CubemapLighting" : "CS_Lighting";
	if (!_content->GetShader(shaderName)->BindShader(_context))
	{
		ErrMsg(std::format("Failed to bind compute shader!"));
		return false;
	}

	_context->CSSetUnorderedAccessViews(0, 1, &targetUAV, nullptr);

	// Bind compute shader resources
	ID3D11ShaderResourceView *srvs[G_BUFFER_COUNT] = { };
	for (size_t i = 0; i < G_BUFFER_COUNT; i++)
		srvs[i] = targetGBuffers->at(i).GetSRV();
	_context->CSSetShaderResources(0, G_BUFFER_COUNT, srvs);

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

	// Bind cubemap texture
	if (!useCubemapShader && _currCubemap != nullptr)
	{
		ID3D11ShaderResourceView *srv = _currCubemap->GetSRV();
		_context->CSSetShaderResources(5, 1, &srv);
	}

	// Bind camera lighting data
	if (!_currMainCamera->BindLightingBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		//return false;
	}

	// Send execution command
	_context->Dispatch(static_cast<UINT>(targetViewport->Width / 8), static_cast<UINT>(targetViewport->Height / 8), 1);

	// Unbind cubemap texture
	if (!useCubemapShader && _currCubemap != nullptr)
	{
		ID3D11ShaderResourceView *nullSRV = nullptr;
		_context->CSSetShaderResources(5, 1, &nullSRV);
	}

	// Unbind spotlight collection
	if (!_currSpotLightCollection->UnbindCSBuffers(_context))
	{
		ErrMsg("Failed to unbind spotlight buffers!");
		return false;
	}

	// Unbind compute shader resources
	memset(srvs, 0, sizeof(srvs));
	_context->CSSetShaderResources(0, G_BUFFER_COUNT, srvs);

	// Unbind render target
	static ID3D11UnorderedAccessView *const nullUAV = nullptr;
	_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

	return true;
}

bool Graphics::RenderGBuffer(const UINT bufferIndex) const
{
	if (bufferIndex >= G_BUFFER_COUNT)
	{
		ErrMsg(std::format("Failed to render g-buffer #{}, index out of range!", bufferIndex));
		return false;
	}

	if (!_content->GetShader("CS_GBuffer")->BindShader(_context))
	{
		ErrMsg("Failed to bind compute shader!");
		return false;
	}

	_context->CSSetUnorderedAccessViews(0, 1, &_uav, nullptr);

	// Bind g-buffer
	ID3D11ShaderResourceView *srv = _gBuffers[bufferIndex].GetSRV();
	_context->CSSetShaderResources(0, 1, &srv);

	// Send execution command
	_context->Dispatch(static_cast<UINT>(_viewport.Width / 8), static_cast<UINT>(_viewport.Height / 8), 1);

	// Unbind compute shader resources
	srv = nullptr;
	_context->CSSetShaderResources(0, 1, &srv);

	static ID3D11UnorderedAccessView *const nullUAV = nullptr;
	_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

	return true;
}

bool Graphics::RenderTransparency(ID3D11RenderTargetView *targetRTV, ID3D11DepthStencilView *targetDSV, const D3D11_VIEWPORT *targetViewport)
{
	ID3D11DepthStencilState *prevStencilState;
	UINT prevStencilRef = 0;
	_context->OMGetDepthStencilState(&prevStencilState, &prevStencilRef);
	_context->OMSetDepthStencilState(_tdss, 0);

	ID3D11BlendState *prevBlendState;
	FLOAT prevBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT prevSampleMask = 0;
	_context->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);

	constexpr float transparentBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_context->OMSetBlendState(_tbs, transparentBlendFactor, 0xffffffff);

	_context->OMSetRenderTargets(1, &targetRTV, targetDSV);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->RSSetViewports(1, targetViewport);
	_context->RSSetState(nullptr); // TODO: Does this work?

	// Bind camera data
	if (!_currViewCamera->BindGeometryBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	// Bind transparency stage resources
	static UINT transparencyInputLayoutID = _content->GetInputLayoutID("IL_Fallback");
	if (_currInputLayoutID != transparencyInputLayoutID)
	{
		_context->IASetInputLayout(_content->GetInputLayout(transparencyInputLayoutID)->GetInputLayout());
		_currInputLayoutID = transparencyInputLayoutID;
	}

	static UINT vsID = _content->GetShaderID("VS_Geometry");
	if (_currVsID != vsID)
	{
		if (!_content->GetShader(vsID)->BindShader(_context))
		{
			ErrMsg("Failed to bind geometry vertex shader!");
			return false;
		}
		_currVsID = vsID;
	}

	static UINT psID = _content->GetShaderID("PS_Transparent");
	if (_currPsID != psID)
	{
		if (!_content->GetShader(psID)->BindShader(_context))
		{
			ErrMsg("Failed to bind transparent pixel shader!");
			return false;
		}
		_currPsID = psID;
	}

	static UINT ssID = _content->GetSamplerID("SS_Fallback");
	if (_currSamplerID != ssID)
	{
		ID3D11SamplerState *const ss = _content->GetSampler(ssID)->GetSamplerState();
		_context->PSSetSamplers(0, 1, &ss);
		_currSamplerID = ssID;
	}

	// Bind global light data
	ID3D11Buffer *const globalLightBuffer = _globalLightBuffer.GetBuffer();
	_context->PSSetConstantBuffers(0, 1, &globalLightBuffer);

	// Bind spotlight collection
	if (!_currSpotLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind spotlight buffers!");
		return false;
	}

	// Bind camera lighting data
	if (!_currMainCamera->BindLightingBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		//return false;
	}

	static UINT defaultNormalID = _content->GetTextureMapID("TexMap_Default_Normal");
	if (_currNormalID != defaultNormalID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultNormalID)->GetSRV();
		_context->PSSetShaderResources(1, 1, &srv);
		_currNormalID = defaultNormalID;
	}

	static UINT defaultSpecularID = _content->GetTextureMapID("TexMap_Default_Specular");
	if (_currSpecularID != defaultSpecularID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultSpecularID)->GetSRV();
		_context->PSSetShaderResources(2, 1, &srv);
		_currSpecularID = defaultSpecularID;
	}

	const MeshD3D11 *loadedMesh = nullptr;
	UINT entity_i = 0;
	for (const auto &[resources, instance] : _currMainCamera->GetTransparentQueue())
	{
		if (static_cast<Entity *>(instance.subject)->GetType() != EntityType::OBJECT)
		{
			ErrMsg(std::format("Skipping depth-rendering for non-object #{}!", entity_i));
			return false;
		}

		// Bind shared geometry resources
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

		if (resources.normalID != CONTENT_LOAD_ERROR)
			if (_currNormalID != resources.normalID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.normalID)->GetSRV();
				_context->PSSetShaderResources(1, 1, &srv);
				_currNormalID = resources.normalID;
			}

		if (resources.specularID != CONTENT_LOAD_ERROR)
			if (_currSpecularID != resources.specularID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.specularID)->GetSRV();
				_context->PSSetShaderResources(2, 1, &srv);
				_currSpecularID = resources.specularID;
			}

		// Bind private entity resources
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

	bool firstEmitter = true;
	entity_i = 0;
	for (const auto &[resources, instance] : _currMainCamera->GetParticleQueue())
	{
		// Bind shared emitter resources
		if (firstEmitter)
		{
			firstEmitter = false;

			_context->IASetInputLayout(nullptr);
			_currInputLayoutID = CONTENT_LOAD_ERROR;

			_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

			const UINT particleVShaderID = _content->GetShaderID("VS_Particle");
			if (_currVsID != particleVShaderID)
			{
				if (!_content->GetShader(particleVShaderID)->BindShader(_context))
				{
					ErrMsg("Failed to bind particle vertex shader!");
					return false;
				}
				_currVsID = particleVShaderID;
			}

			const UINT particlePShaderID = _content->GetShaderID("PS_Particle");
			if (_currPsID != particlePShaderID)
			{
				if (!_content->GetShader(particlePShaderID)->BindShader(_context))
				{
					ErrMsg("Failed to bind particle pixel shader!");
					return false;
				}
				_currPsID = particlePShaderID;
			}

			if (!_content->GetShader(_content->GetShaderID("GS_Billboard"))->BindShader(_context))
			{
				ErrMsg("Failed to bind billboard geometry shader!");
				return false;
			}
		}

		if (resources.texID != CONTENT_LOAD_ERROR)
			if (_currTexID != resources.texID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTexture(resources.texID)->GetSRV();
				_context->PSSetShaderResources(0, 1, &srv);
				_currTexID = resources.texID;
			}

		// Bind private emitter resources
		if (!static_cast<Emitter *>(instance.subject)->BindBuffers(_context))
		{
			ErrMsg("Failed to bind emitter buffers!");
			return false;
		}

		if (!static_cast<Emitter *>(instance.subject)->PerformDrawCall(_context))
		{
			ErrMsg("Failed to perform emitter draw call!");
			return false;
		}

		entity_i++;
	}

	if (!firstEmitter)
	{
		// Unbind particle resources
		_context->GSSetShader(nullptr, nullptr, 0);

		ID3D11ShaderResourceView *nullSRV = nullptr;
		_context->VSSetShaderResources(0, 1, &nullSRV);
	}

	// Unbind spotlight collection
	if (!_currSpotLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind spotlight buffers!");
		return false;
	}

	_context->OMSetBlendState(prevBlendState, prevBlendFactor, prevSampleMask);
	_context->OMSetDepthStencilState(prevStencilState, prevStencilRef);

	static ID3D11RenderTargetView *const nullRTV = nullptr;
	_context->OMSetRenderTargets(1, &nullRTV, nullptr);

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

bool Graphics::RenderUI(Time &time)
{
	char fps[8]{};
	snprintf(fps, sizeof(fps), "%.2f", 1.0f / time.deltaTime);
	ImGui::Text(std::format("fps: {}", fps).c_str());

	static float minFPS = FLT_MAX;
	if (minFPS > 1.0f / time.deltaTime)
		minFPS = 1.0f / time.deltaTime;
	ImGui::Text(std::format("Drop: {}", minFPS).c_str());

	if (ImGui::Button("Reset FPS"))
		minFPS = 1.0f / time.deltaTime;

	std::string currRenderOutput;
	if		(_renderOutput == 1) currRenderOutput = "Positions";
	else if (_renderOutput == 2) currRenderOutput = "Colors";
	else if (_renderOutput == 3) currRenderOutput = "Normals";
	else						 currRenderOutput = "Default";

	if (ImGui::Button(std::format("Render Output: {}", currRenderOutput).c_str()))
		_renderOutput++;

	if (ImGui::Button(std::format("Reflections: {}", _updateCubemap ? "Enabled" : "Disabled").c_str()))
		_updateCubemap = !_updateCubemap;

	ImGui::Text(std::format("Main Draws: {}", _currMainCamera->GetCullCount()).c_str());
	for (UINT i = 0; i < _currSpotLightCollection->GetNrOfLights(); i++)
	{
		const CameraD3D11 *spotlightCamera = _currSpotLightCollection->GetLightCamera(i);
		ImGui::Text(std::format("Spotlight #{} Draws: {}", i, spotlightCamera->GetCullCount()).c_str());
	}

	return true;
}

bool Graphics::EndUIRender() const
{
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	static ID3D11RenderTargetView *const nullRTV = nullptr;
	_context->OMSetRenderTargets(1, &nullRTV, nullptr);

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
	_currMainCamera->ResetRenderQueue();

	for (UINT i = 0; i < _currSpotLightCollection->GetNrOfLights(); i++)
		_currSpotLightCollection->GetLightCamera(i)->ResetRenderQueue();

	if (_currCubemap != nullptr)
		if (_currCubemap->GetUpdate())
			for (UINT i = 0; i < 6; i++)
				_currCubemap->GetCamera(i)->ResetRenderQueue();
	_currCubemap = nullptr;

	_currInputLayoutID	= CONTENT_LOAD_ERROR;
	_currMeshID			= CONTENT_LOAD_ERROR;
	_currVsID			= CONTENT_LOAD_ERROR;
	_currPsID			= CONTENT_LOAD_ERROR;
	_currTexID			= CONTENT_LOAD_ERROR;
	_currNormalID		= CONTENT_LOAD_ERROR;
	_currSpecularID		= CONTENT_LOAD_ERROR;
	_currReflectiveID	= CONTENT_LOAD_ERROR;

	_isRendering = false;
	return true;
}
