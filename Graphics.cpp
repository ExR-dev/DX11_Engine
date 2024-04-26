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
	if (_shadowRasterizer != nullptr)
		_shadowRasterizer->Release();

	if (_wireframeRasterizer != nullptr)
		_wireframeRasterizer->Release();

	if (_defaultRasterizer != nullptr)
		_defaultRasterizer->Release();

	if (_tdss != nullptr)
		_tdss->Release();

	if (_ndss != nullptr)
		_ndss->Release();

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
			_swapChain, _rtv, _dsTexture, _dsView, _uav, _tbs, _ndss, _tdss, _viewport))
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

	D3D11_RASTERIZER_DESC rasterizerDesc = { };
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &_defaultRasterizer)))
	{
		ErrMsg("Failed to create default rasterizer state!");
		return false;
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &_wireframeRasterizer)))
	{
		ErrMsg("Failed to create wireframe rasterizer state!");
		return false;
	}

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = -1;
	rasterizerDesc.DepthBiasClamp = -0.01f;
	rasterizerDesc.SlopeScaledDepthBias = -3.0f;
	rasterizerDesc.DepthClipEnable = false;

	if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &_shadowRasterizer)))
	{
		ErrMsg("Failed to create shadow rasterizer state!");
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

bool Graphics::SetSpotlightCollection(SpotLightCollectionD3D11 *spotlights)
{
	if (spotlights == nullptr)
	{
		ErrMsg("Failed to set spotlight collection, collection is nullptr!");
		return false;
	}

	_currSpotLightCollection = spotlights;
	return true;
}

bool Graphics::SetDirlightCollection(DirLightCollectionD3D11 *dirlights)
{
	if (dirlights == nullptr)
	{
		ErrMsg("Failed to set directional light collection, collection is nullptr!");
		return false;
	}

	_currDirLightCollection = dirlights;
	return true;
}

bool Graphics::SetPointlightCollection(PointLightCollectionD3D11 *pointlights)
{
	if (pointlights == nullptr)
	{
		ErrMsg("Failed to set pointlight collection, collection is nullptr!");
		return false;
	}

	_currPointLightCollection = pointlights;
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

	_context->OMSetDepthStencilState(_ndss, 0);

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

		if (_renderTransparency)
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

	_currInputLayoutID = CONTENT_LOAD_ERROR;
	_currMeshID = CONTENT_LOAD_ERROR;
	_currVsID = CONTENT_LOAD_ERROR;
	_currPsID = CONTENT_LOAD_ERROR;
	_currTexID = CONTENT_LOAD_ERROR;
	_currNormalID = CONTENT_LOAD_ERROR;
	_currSpecularID = CONTENT_LOAD_ERROR;
	_currReflectiveID = CONTENT_LOAD_ERROR;
	_currAmbientID = CONTENT_LOAD_ERROR;
	_currHeightID = CONTENT_LOAD_ERROR;

	return true;
}


bool Graphics::RenderSpotlights()
{
	if (_currSpotLightCollection == nullptr)
	{
		ErrMsg("Failed to render spotlights, current spotlight collection is nullptr!");
		return false;
	}

	_context->RSSetViewports(1, &_currSpotLightCollection->GetViewport());

	const MeshD3D11 *loadedMesh = nullptr;

	const UINT spotLightCount = _currSpotLightCollection->GetNrOfLights();
	for (UINT spotlight_i = 0; spotlight_i < spotLightCount; spotlight_i++)
	{
		// Skip rendering if disabled
		if (!_currSpotLightCollection->GetLightEnabled(spotlight_i))
			continue;

		ID3D11DepthStencilView *dsView = _currSpotLightCollection->GetShadowMapDSV(spotlight_i);
		_context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 0.0f, 0);
		_context->OMSetRenderTargets(0, nullptr, dsView);

		// Bind shadow-camera data
		const CameraD3D11 *spotlightCamera = _currSpotLightCollection->GetLightCamera(spotlight_i);

		if (!spotlightCamera->BindShadowCasterBuffers(_context))
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

			const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
			for (UINT submesh_i = 0; submesh_i < subMeshCount; submesh_i++)
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

	return true;
}

bool Graphics::RenderDirlights()
{
	if (_currDirLightCollection == nullptr)
	{
		ErrMsg("Failed to render directional lights, current directional light collection is nullptr!");
		return false;
	}

	_context->RSSetViewports(1, &_currDirLightCollection->GetViewport());

	const MeshD3D11 *loadedMesh = nullptr;

	const UINT dirLightCount = _currDirLightCollection->GetNrOfLights();
	for (UINT dirlight_i = 0; dirlight_i < dirLightCount; dirlight_i++)
	{
		// Skip rendering if disabled
		if (!_currDirLightCollection->GetLightEnabled(dirlight_i))
			continue;

		ID3D11DepthStencilView *dsView = _currDirLightCollection->GetShadowMapDSV(dirlight_i);
		_context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 0.0f, 0);
		_context->OMSetRenderTargets(0, nullptr, dsView);

		// Bind shadow-camera data
		const CameraD3D11 *dirlightCamera = _currDirLightCollection->GetLightCamera(dirlight_i);

		if (!dirlightCamera->BindShadowCasterBuffers(_context))
		{
			ErrMsg(std::format("Failed to bind shadow-camera buffers for directional light #{}!", dirlight_i));
			return false;
		}

		UINT entity_i = 0;
		for (const auto &[resources, instance] : dirlightCamera->GetGeometryQueue())
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

			const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
			for (UINT submesh_i = 0; submesh_i < subMeshCount; submesh_i++)
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

	return true;
}

bool Graphics::RenderPointlights()
{
	if (_currPointLightCollection == nullptr)
	{
		ErrMsg("Failed to render pointlights, current pointlight collection is nullptr!");
		return false;
	}

	_context->RSSetViewports(1, &_currPointLightCollection->GetViewport());

	const MeshD3D11 *loadedMesh = nullptr;

	const UINT pointlightCount = _currPointLightCollection->GetNrOfLights();
	for (UINT pointlight_i = 0; pointlight_i < pointlightCount; pointlight_i++)
		for (UINT camera_i = 0; camera_i < 6; camera_i++)
		{
			// Skip rendering if disabled
			if (!_currPointLightCollection->IsEnabled(pointlight_i, camera_i))
				continue;

			ID3D11DepthStencilView *dsView = _currPointLightCollection->GetShadowMapDSV(pointlight_i, camera_i);
			_context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 0.0f, 0);
			_context->OMSetRenderTargets(0, nullptr, dsView);

			// Bind shadow-camera data
			const CameraD3D11 *pointlightCamera = _currPointLightCollection->GetLightCamera(pointlight_i, camera_i);

			if (!pointlightCamera->BindShadowCasterBuffers(_context))
			{
				ErrMsg(std::format("Failed to bind shadow-camera buffers for pointlight #{} camera #{}!", pointlight_i, camera_i));
				return false;
			}

			UINT entity_i = 0;
			for (const auto &[resources, instance] : pointlightCamera->GetGeometryQueue())
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

				const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
				for (UINT submesh_i = 0; submesh_i < subMeshCount; submesh_i++)
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

	return true;
}

bool Graphics::RenderShadowCasters()
{
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
	_context->RSSetState(_shadowRasterizer);

	if (!RenderSpotlights())
	{
		ErrMsg("Failed to render spotlights!");
		return false;
	}

	if (!RenderDirlights())
	{
		ErrMsg("Failed to render directional lights!");
		return false;
	}

	if (!RenderPointlights())
	{
		ErrMsg("Failed to render pointlights!");
		return false;
	}

	// Unbind render target
	_context->OMSetRenderTargets(0, nullptr, nullptr);

	_context->RSSetState(_defaultRasterizer);

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

	_context->ClearDepthStencilView(targetDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	_context->RSSetViewports(1, targetViewport);
	_context->RSSetState(_wireframe ? _wireframeRasterizer : _defaultRasterizer);

	// Bind camera data
	if (!_currMainCamera->BindMainBuffers(_context))
	{
		ErrMsg("Failed to bind main camera buffers!");
		return false;
	}

	if (!_currViewCamera->BindViewBuffers(_context))
	{
		ErrMsg("Failed to bind view camera buffers!");
		return false;
	}

	// Bind geometry stage resources
	static UINT geometryInputLayoutID = _content->GetInputLayoutID("IL_Fallback");
	_context->IASetInputLayout(_content->GetInputLayout(geometryInputLayoutID)->GetInputLayout());
	_currInputLayoutID = geometryInputLayoutID;

	static UINT vsID = _content->GetShaderID("VS_Geometry");
	if (!_content->GetShader(vsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind geometry vertex shader!");
		return false;
	}
	_currVsID = vsID;

	static UINT hsID = _content->GetShaderID("HS_LOD");
	if (!_content->GetShader(hsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD hull shader!");
		return false;
	}

	static UINT dsID = _content->GetShaderID("DS_LOD");
	if (!_content->GetShader(dsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD domain shader!");
		return false;
	}

	static UINT psID = _content->GetShaderID("PS_Geometry");
	if (!_content->GetShader(psID)->BindShader(_context))
	{
		ErrMsg("Failed to bind geometry pixel shader!");
		return false;
	}
	_currPsID = psID;

	static UINT ssID = _content->GetSamplerID("SS_Fallback");
	if (_currSamplerID != ssID)
	{
		ID3D11SamplerState *const ss = _content->GetSampler(ssID)->GetSamplerState();
		_context->PSSetSamplers(0, 1, &ss);
		_context->DSSetSamplers(0, 1, &ss);
		_currSamplerID = ssID;
	}

	static UINT defaultNormalID = _content->GetTextureMapID("TexMap_Default_Normal");
	ID3D11ShaderResourceView *srv = _content->GetTextureMap(defaultNormalID)->GetSRV();
	_context->PSSetShaderResources(1, 1, &srv);
	_currNormalID = defaultNormalID;

	static UINT defaultSpecularID = _content->GetTextureMapID("TexMap_Default_Specular");
	srv = _content->GetTextureMap(defaultSpecularID)->GetSRV();
	_context->PSSetShaderResources(2, 1, &srv);
	_currSpecularID = defaultSpecularID;

	static UINT defaultReflectiveID = _content->GetTextureMapID("TexMap_Default_Reflective");
	srv = _content->GetTextureMap(defaultReflectiveID)->GetSRV();
	_context->PSSetShaderResources(3, 1, &srv);
	_currReflectiveID = defaultReflectiveID;

	static UINT defaultAmbientID = _content->GetTextureID("Tex_Ambient");
	srv = _content->GetTexture(defaultAmbientID)->GetSRV();
	_context->PSSetShaderResources(4, 1, &srv);
	_currAmbientID = defaultAmbientID;

	static UINT defaultHeightID = _content->GetTextureMapID("TexMap_Default_Height");
	srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
	_context->DSSetShaderResources(0, 1, &srv);
	_currHeightID = defaultHeightID;

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
			srv = _content->GetTexture(resources.texID)->GetSRV();
			_context->PSSetShaderResources(0, 1, &srv);
			_currTexID = resources.texID;
		}

		if (resources.normalID != CONTENT_LOAD_ERROR)
			if (_currNormalID != resources.normalID)
			{
				srv = _content->GetTextureMap(resources.normalID)->GetSRV();
				_context->PSSetShaderResources(1, 1, &srv);
				_currNormalID = resources.normalID;
			}

		if (resources.specularID != CONTENT_LOAD_ERROR)
			if (_currSpecularID != resources.specularID)
			{
				srv = _content->GetTextureMap(resources.specularID)->GetSRV();
				_context->PSSetShaderResources(2, 1, &srv);
				_currSpecularID = resources.specularID;
			}

		if (resources.reflectiveID != CONTENT_LOAD_ERROR)
			if (_currReflectiveID != resources.reflectiveID)
			{
				srv = _content->GetTextureMap(resources.reflectiveID)->GetSRV();
				_context->PSSetShaderResources(3, 1, &srv);
				_currReflectiveID = resources.reflectiveID;
			}

		if (resources.ambientID != CONTENT_LOAD_ERROR)
			if (_currAmbientID != resources.ambientID)
			{
				srv = _content->GetTexture(resources.ambientID)->GetSRV();
				_context->PSSetShaderResources(4, 1, &srv);
				_currAmbientID = resources.ambientID;
			}

		if (resources.heightID != CONTENT_LOAD_ERROR)
		{
			if (_currHeightID != resources.heightID)
			{
				srv = _content->GetTextureMap(resources.heightID)->GetSRV();
				_context->DSSetShaderResources(0, 1, &srv);
				_currHeightID = resources.heightID;
			}
		}
		else if (_currHeightID != defaultHeightID)
		{
			srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
			_context->DSSetShaderResources(0, 1, &srv);
			_currHeightID = defaultHeightID;
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

		const UINT
			prevTexID = _currTexID,
			prevAmbientID = _currAmbientID,
			prevSpecularID = _currSpecularID;

		const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
		for (UINT i = 0; i < subMeshCount; i++)
		{
			// Bind sub-mesh material textures if defined
			std::string path = loadedMesh->GetDiffusePath(i);
			if (path != "")
			{
				const UINT id = _content->GetTextureIDByPath(path);
				if (id != CONTENT_LOAD_ERROR && id != _currTexID)
				{
					srv = _content->GetTexture(id)->GetSRV();
					_context->PSSetShaderResources(0, 1, &srv);
					_currTexID = id;
				}
			}
			else if (prevTexID != _currTexID && prevTexID != CONTENT_LOAD_ERROR)
			{
				srv = _content->GetTexture(prevTexID)->GetSRV();
				_context->PSSetShaderResources(0, 1, &srv);
				_currTexID = prevTexID;
			}

			path = loadedMesh->GetAmbientPath(i);
			if (path != "")
			{
				const UINT id = _content->GetTextureIDByPath(path);
				if (id != CONTENT_LOAD_ERROR && id != _currAmbientID)
				{
					srv = _content->GetTexture(id)->GetSRV();
					_context->PSSetShaderResources(4, 1, &srv);
					_currAmbientID = id;
				}
			}
			else if (prevAmbientID != _currAmbientID && prevAmbientID != CONTENT_LOAD_ERROR)
			{
				srv = _content->GetTexture(prevAmbientID)->GetSRV();
				_context->PSSetShaderResources(4, 1, &srv);
				_currAmbientID = prevAmbientID;
			}

			path = loadedMesh->GetSpecularPath(i);
			if (path != "")
			{
				const UINT id = _content->GetTextureMapIDByPath(path, TextureType::SPECULAR);
				if (id != CONTENT_LOAD_ERROR && id != _currSpecularID)
				{
					srv = _content->GetTextureMap(id)->GetSRV();
					_context->PSSetShaderResources(2, 1, &srv);
					_currSpecularID = id;
				}
			}
			else if (prevSpecularID != _currSpecularID && prevSpecularID != CONTENT_LOAD_ERROR)
			{
				srv = _content->GetTextureMap(prevSpecularID)->GetSRV();
				_context->PSSetShaderResources(2, 1, &srv);
				_currSpecularID = prevSpecularID;
			}

			ID3D11Buffer *const specularBuffer = loadedMesh->GetSpecularBuffer(i);
			_context->PSSetConstantBuffers(1, 1, &specularBuffer);

			if (!loadedMesh->PerformSubMeshDrawCall(_context, i))
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, i));
				return false;
			}
		}

		entity_i++;
	}

	// Unbind tesselation shaders
	_context->HSSetShader(nullptr, nullptr, 0);
	_context->DSSetShader(nullptr, nullptr, 0);

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


	// Bind spotlight collection
	if (!_currSpotLightCollection->BindCSBuffers(_context))
	{
		ErrMsg("Failed to bind spotlight buffers!");
		return false;
	}

	// Bind directional light collection
	if (!_currDirLightCollection->BindCSBuffers(_context))
	{
		ErrMsg("Failed to bind directional light buffers!");
		return false;
	}

	// Bind pointlight collection
	if (!_currPointLightCollection->BindCSBuffers(_context))
	{
		ErrMsg("Failed to bind pointlight buffers!");
		return false;
	}

	// Bind cubemap texture
	if (!useCubemapShader && _currCubemap != nullptr)
	{
		ID3D11ShaderResourceView *srv = _currCubemap->GetSRV();
		_context->CSSetShaderResources(10, 1, &srv);
	}

	static ID3D11SamplerState *const ss = _content->GetSampler("SS_Clamp")->GetSamplerState();
	_context->CSSetSamplers(0, 1, &ss);

	// Bind camera lighting data
	if (!_currMainCamera->BindLightingBuffers(_context))
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	// Send execution command
	_context->Dispatch(static_cast<UINT>(targetViewport->Width / 8), static_cast<UINT>(targetViewport->Height / 8), 1);

	// Unbind cubemap texture
	if (!useCubemapShader && _currCubemap != nullptr)
	{
		ID3D11ShaderResourceView *nullSRV = nullptr;
		_context->CSSetShaderResources(10, 1, &nullSRV);
	}

	// Unbind pointlight collection
	if (!_currPointLightCollection->UnbindCSBuffers(_context))
	{
		ErrMsg("Failed to unbind pointlight buffers!");
		return false;
	}

	// Unbind directional light collection
	if (!_currDirLightCollection->UnbindCSBuffers(_context))
	{
		ErrMsg("Failed to unbind directional light buffers!");
		return false;
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
	_context->OMSetDepthStencilState(_tdss, 0);

	ID3D11BlendState *prevBlendState;
	FLOAT prevBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT prevSampleMask = 0;
	_context->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);

	constexpr float transparentBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_context->OMSetBlendState(_tbs, transparentBlendFactor, 0xffffffff);

	_context->OMSetRenderTargets(1, &targetRTV, targetDSV);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST); // Enabled tessellation, otherwise D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	_context->RSSetViewports(1, targetViewport);
	_context->RSSetState(_wireframe ? _wireframeRasterizer : _defaultRasterizer);

	// Bind camera data
	if (!_currMainCamera->BindMainBuffers(_context))
	{
		ErrMsg("Failed to bind main camera buffers!");
		return false;
	}

	if (!_currViewCamera->BindViewBuffers(_context))
	{
		ErrMsg("Failed to bind view camera buffers!");
		return false;
	}

	// Bind billboard camera data
	if (!_currViewCamera->BindTransparentBuffers(_context))
	{
		ErrMsg("Failed to bind billboard camera buffers!");
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

	static UINT hsID = _content->GetShaderID("HS_LOD");
	if (!_content->GetShader(hsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD hull shader!");
		return false;
	}

	static UINT dsID = _content->GetShaderID("DS_LOD");
	if (!_content->GetShader(dsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD domain shader!");
		return false;
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

	static UINT ssID = _content->GetSamplerID("SS_Clamp");
	if (_currSamplerID != ssID)
	{
		ID3D11SamplerState *const ss = _content->GetSampler(ssID)->GetSamplerState();
		_context->PSSetSamplers(0, 1, &ss);
		_context->DSSetSamplers(0, 1, &ss);
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

	// Bind directional light collection
	if (!_currDirLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind directional light buffers!");
		return false;
	}

	// Bind pointlight collection
	if (!_currPointLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind pointlight buffers!");
		return false;
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

	static UINT defaultHeightID = _content->GetTextureMapID("TexMap_Default_Height");
	if (_currHeightID != defaultHeightID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
		_context->DSSetShaderResources(0, 1, &srv);
		_currHeightID = defaultHeightID;
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

		if (resources.heightID != CONTENT_LOAD_ERROR)
		{
			if (_currHeightID != resources.heightID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.heightID)->GetSRV();
				_context->DSSetShaderResources(0, 1, &srv);
				_currHeightID = resources.heightID;
			}
		}
		else if (_currHeightID != defaultHeightID)
		{
			ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
			_context->DSSetShaderResources(0, 1, &srv);
			_currHeightID = defaultHeightID;
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

		const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
		for (UINT i = 0; i < subMeshCount; i++)
		{
			if (!loadedMesh->PerformSubMeshDrawCall(_context, i))
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, i));
				return false;
			}
		}

		entity_i++;
	}

	// Unbind tesselation shaders
	_context->HSSetShader(nullptr, nullptr, 0);
	_context->DSSetShader(nullptr, nullptr, 0);

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

	// Unbind pointlight collection
	if (!_currPointLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind pointlight buffers!");
		return false;
	}

	// Unbind directional light collection
	if (!_currDirLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind directional light buffers!");
		return false;
	}

	// Unbind spotlight collection
	if (!_currSpotLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind spotlight buffers!");
		return false;
	}

	// Reset blend state
	_context->OMSetBlendState(prevBlendState, prevBlendFactor, prevSampleMask);
	_context->OMSetDepthStencilState(_ndss, 0);

	// Unbind render target
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
	if		(_renderOutput == 1) currRenderOutput = "Position";
	else if (_renderOutput == 2) currRenderOutput = "Normal";
	else if (_renderOutput == 3) currRenderOutput = "Ambient";
	else if (_renderOutput == 4) currRenderOutput = "Diffuse";
	else						 currRenderOutput = "Default";

	if (ImGui::Button(std::format("Render Output: {}", currRenderOutput).c_str()))
		_renderOutput++;

	if (ImGui::Button(std::format("Wireframe Mode: {}", _wireframe ? "Enabled" : "Disabled").c_str()))
		_wireframe = !_wireframe;

	if (ImGui::Button(std::format("Reflections: {}", _updateCubemap ? "Enabled" : "Disabled").c_str()))
		_updateCubemap = !_updateCubemap;

	if (ImGui::Button(std::format("Transparency: {}", _renderTransparency ? "Enabled" : "Disabled").c_str()))
		_renderTransparency = !_renderTransparency;

	ImGui::Text(std::format("Main Draws: {}", _currMainCamera->GetCullCount()).c_str());
	for (UINT i = 0; i < _currSpotLightCollection->GetNrOfLights(); i++)
	{
		const CameraD3D11 *spotlightCamera = _currSpotLightCollection->GetLightCamera(i);
		ImGui::Text(std::format("Spotlight #{} Draws: {}", i, spotlightCamera->GetCullCount()).c_str());
	}

	for (UINT i = 0; i < _currDirLightCollection->GetNrOfLights(); i++)
	{
		const CameraD3D11 *dirlightCamera = _currDirLightCollection->GetLightCamera(i);
		ImGui::Text(std::format("Dirlight #{} Draws: {}", i, dirlightCamera->GetCullCount()).c_str());
	}

	for (UINT i = 0; i < _currPointLightCollection->GetNrOfLights(); i++)
		for (UINT j = 0; j < 6; j++)
		{
			const CameraD3D11 *pointlightCamera = _currPointLightCollection->GetLightCamera(i, j);
			ImGui::Text(std::format("Pointlight #{}:{} Draws: {}", i, j, pointlightCamera->GetCullCount()).c_str());
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

	for (UINT i = 0; i < _currDirLightCollection->GetNrOfLights(); i++)
		_currDirLightCollection->GetLightCamera(i)->ResetRenderQueue();

	for (UINT i = 0; i < _currPointLightCollection->GetNrOfLights(); i++)
		for (UINT j = 0; j < 6; j++)
			_currPointLightCollection->GetLightCamera(i, j)->ResetRenderQueue();

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
	_currAmbientID		= CONTENT_LOAD_ERROR;
	_currHeightID		= CONTENT_LOAD_ERROR;

	_isRendering = false;
	return true;
}
