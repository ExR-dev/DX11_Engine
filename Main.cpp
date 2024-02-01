#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <chrono>
#include <DirectXMath.h>

#include "WindowHelper.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"

using namespace DirectX;


void Render(
	ID3D11DeviceContext *immediateContext, ID3D11RenderTargetView *rtv,
	ID3D11DepthStencilView *dsView, D3D11_VIEWPORT &viewport, 
	ID3D11VertexShader *vShader, ID3D11PixelShader *pShader, 
	ID3D11InputLayout *inputLayout, ID3D11Buffer *vertexBuffer, 
	ID3D11Buffer *matrixBuffer, ID3D11Buffer *lightingBuffer,
	ID3D11ShaderResourceView *resourceView, ID3D11SamplerState *samplerState)
{
	constexpr float clearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	immediateContext->ClearRenderTargetView(rtv, clearColour);
	immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	constexpr UINT stride = sizeof(SimpleVertex);
	constexpr UINT offset = 0;

	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	immediateContext->IASetInputLayout(inputLayout);
	immediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediateContext->PSSetConstantBuffers(0, 1, &lightingBuffer);
	immediateContext->PSSetShaderResources(0, 1, &resourceView);
	immediateContext->PSSetSamplers(0, 1, &samplerState);

	immediateContext->VSSetShader(vShader, nullptr, 0); 
	immediateContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	immediateContext->RSSetViewports(1, &viewport);
	immediateContext->PSSetShader(pShader, nullptr, 0);

	immediateContext->OMSetRenderTargets(1, &rtv, dsView);

	immediateContext->Draw(6, 0);
}

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	constexpr UINT WIDTH = 900;
	constexpr UINT HEIGHT = 900;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	ID3D11Device *device;
	ID3D11DeviceContext	*immediateContext;
	IDXGISwapChain *swapChain;
	ID3D11RenderTargetView *rtv;
	ID3D11Texture2D *dsTexture;
	ID3D11DepthStencilView *dsView;
	D3D11_VIEWPORT viewport;
	ID3D11VertexShader *vShader;
	ID3D11PixelShader *pShader;
	ID3D11InputLayout *inputLayout;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *matrixBuffer;
	ID3D11Buffer *lightingBuffer;
	ID3D11Texture2D *texture2D;
	ID3D11ShaderResourceView *resourceView;
	ID3D11SamplerState *samplerState;

	if (!SetupD3D11(WIDTH, HEIGHT, window, device, immediateContext, swapChain, rtv, dsTexture, dsView, viewport))
	{
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return -1;
	}

	if (!SetupPipeline(device, vertexBuffer, matrixBuffer, lightingBuffer, texture2D, resourceView, samplerState, vShader, pShader, inputLayout))
	{
		std::cerr << "Failed to setup pipeline!" << std::endl;
		return -1;
	}

	MatrixBufferData matrixBufferData = { };
	LightingBufferData lightingBufferData = {
		{0.0f, 0.5f, -2.5f, 1.0f}, // Camera position
		{3.0f, 0.0f, -3.0f, 1.0f}, // Light position

		{0.75f, 0.9f, 1.0f, 0.05f}, // Ambient
		{1.0f, 1.0f, 1.0f, 10.0f}, // Diffuse
		{15.0f, 15.0f, 15.0f, 256.0f}, // Specular
	};

	GTime time = { };
	const auto programStart = std::chrono::high_resolution_clock::now();
	auto lFrameStart = programStart;
	auto frameStart = programStart;

	MSG msg = { };
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		lFrameStart = frameStart;
		frameStart = std::chrono::high_resolution_clock::now();

		time.lifetime = (double)(std::chrono::duration_cast<std::chrono::microseconds>(frameStart - programStart).count()) * 0.000001;
		time.deltatime = (double)(std::chrono::duration_cast<std::chrono::microseconds>(frameStart - lFrameStart).count()) * 0.000001;

		XMVECTOR camPos = { lightingBufferData.camPos[0], lightingBufferData.camPos[1], lightingBufferData.camPos[2]};
		XMVECTOR camDir = XMVector3Normalize(-camPos);

		const XMMATRIX viewProjMatrix = 
			XMMatrixLookAtLH(camPos, camPos + camDir, {0, 1, 0}) *
			XMMatrixPerspectiveFovLH(45.0f * 0.0174533f, (float)WIDTH / HEIGHT, 0.1f, 10.0f);

		const XMMATRIX worldMatrix = 
			XMMatrixRotationY(-time.lifetime / 1.25f) *
			XMMatrixTranslation(0, 0, 0);

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		XMStoreFloat4x4(&matrixBufferData.viewProjMatrix, XMMatrixTranspose(viewProjMatrix));
		XMStoreFloat4x4(&matrixBufferData.worldMatrix, XMMatrixTranspose(worldMatrix));

		if (!UpdateMatrixBuffer(immediateContext, matrixBuffer, matrixBufferData))
		{
			std::cerr << "Failed to update matrix buffer!" << std::endl;
			return -1;
		}

		if (!UpdateLightingBuffer(immediateContext, lightingBuffer, lightingBufferData))
		{
			std::cerr << "Failed to update lighting buffer!" << std::endl;
			return -1;
		}

		Render(
			immediateContext, rtv, 
			dsView, viewport, 
			vShader, pShader, 
			inputLayout, vertexBuffer, 
			matrixBuffer, lightingBuffer,
			resourceView, samplerState
		);
		swapChain->Present(0, 0);
	}

	samplerState->Release();
	resourceView->Release();
	texture2D->Release();
	lightingBuffer->Release();
	matrixBuffer->Release();
	vertexBuffer->Release();
	inputLayout->Release();
	pShader->Release();
	vShader->Release();
	dsView->Release();
	dsTexture->Release();
	rtv->Release();
	swapChain->Release();
	immediateContext->Release();
	device->Release();

	return 0;
}