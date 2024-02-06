
#include <Windows.h>
#include <iostream>
#include <d3d11.h>

#include "WindowHelper.h"

#include "Game.h"
#include "Time.h"










#include "D3D11Helper.h"
#include "PipelineHelper.h"

using namespace DirectX;


struct MeshData
{
	ID3D11VertexShader *vShader;
	ID3D11PixelShader *pShader;
	ID3D11InputLayout *inputLayout;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *matrixBuffer;
	ID3D11Buffer *lightingBuffer;
	ID3D11Texture2D *texture2D;
	ID3D11ShaderResourceView *resourceView;
	ID3D11SamplerState *samplerState;

	MatrixBufferData matrixBufferData = { };
	LightingBufferData lightingBufferData = {
		{0.0f, 0.5f, -2.5f, 1.0f}, // Camera position
		{3.0f, 0.0f, -3.0f, 1.0f}, // Light position

		{0.75f, 0.9f, 1.0f, 0.05f}, // Ambient
		{1.0f, 1.0f, 1.0f, 10.0f}, // Diffuse
		{1.0f, 1.0f, 1.0f, 256.0f}, // Specular
	};
};



void Deprecated_Render(
	ID3D11DeviceContext *immediateContext, ID3D11RenderTargetView *rtv,
	ID3D11DepthStencilView *dsView, D3D11_VIEWPORT &viewport,
	ID3D11VertexShader *vShader, ID3D11PixelShader *pShader,
	ID3D11InputLayout *inputLayout, ID3D11Buffer *vertexBuffer,
	ID3D11Buffer *matrixBuffer, ID3D11Buffer *lightingBuffer,
	ID3D11ShaderResourceView *resourceView, ID3D11SamplerState *samplerState)
{
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


int Deprecated_wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	constexpr UINT WIDTH = 900;
	constexpr UINT HEIGHT = 900;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	Time time = Time();

	ID3D11Device *device;
	ID3D11DeviceContext *immediateContext;
	IDXGISwapChain *swapChain;
	ID3D11RenderTargetView *rtv;
	ID3D11Texture2D *dsTexture;
	ID3D11DepthStencilView *dsView;
	D3D11_VIEWPORT viewport;

	MeshData mesh1, mesh2;

	if (!SetupD3D11(WIDTH, HEIGHT, window, device, immediateContext, swapChain, rtv, dsTexture, dsView, viewport))
	{
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return -1;
	}

	if (!SetupPipeline(device, mesh1.vertexBuffer, mesh1.matrixBuffer, mesh1.lightingBuffer, mesh1.texture2D, mesh1.resourceView, mesh1.samplerState, mesh1.vShader, mesh1.pShader, mesh1.inputLayout))
	{
		std::cerr << "Failed to setup mesh1 pipeline!" << std::endl;
		return -1;
	}

	if (!SetupPipeline(device, mesh2.vertexBuffer, mesh2.matrixBuffer, mesh2.lightingBuffer, mesh2.texture2D, mesh2.resourceView, mesh2.samplerState, mesh2.vShader, mesh2.pShader, mesh2.inputLayout))
	{
		std::cerr << "Failed to setup mesh2 pipeline!" << std::endl;
		return -1;
	}

	mesh2.lightingBufferData = {
		{0.0f, 0.5f, -2.5f, 1.0f}, // Camera position
		{2.0f, 0.5f, -2.0f, 1.0f}, // Light position

		{0.0f, 1.0f, 0.0f, 0.15f}, // Ambient
		{1.0f, 0.0f, 0.0f, 25.0f}, // Diffuse
		{0.0f, 0.0f, 1.0f, 128.0f}, // Specular
	};

	MSG msg{ };
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		time.Update();

		XMVECTOR camPos = { mesh1.lightingBufferData.camPos[0], mesh1.lightingBufferData.camPos[1], mesh1.lightingBufferData.camPos[2] };
		XMVECTOR camDir = XMVector3Normalize(-camPos);

		XMMATRIX viewProjMatrix =
			XMMatrixLookAtLH(camPos, camPos + camDir, { 0, 1, 0 }) *
			XMMatrixPerspectiveFovLH(45.0f * 0.0174533f, (float)WIDTH / HEIGHT, 0.1f, 10.0f);

		XMMATRIX worldMatrix1 =
			XMMatrixRotationY(-time.time / 1.25f) *
			XMMatrixTranslation(0, 0, 0);

		XMMATRIX worldMatrix2 =
			XMMatrixScaling(0.9f, 0.5f, 1.5f) *
			XMMatrixRotationRollPitchYaw(time.time * 0.76274f, -time.time * 0.3416f, time.time * 0.5384f) *
			XMMatrixTranslation(1.0f, 0.2f, 1);

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DirectX::XMStoreFloat4x4(&mesh1.matrixBufferData.viewProjMatrix, XMMatrixTranspose(viewProjMatrix));
		DirectX::XMStoreFloat4x4(&mesh1.matrixBufferData.worldMatrix, XMMatrixTranspose(worldMatrix1));

		DirectX::XMStoreFloat4x4(&mesh2.matrixBufferData.viewProjMatrix, XMMatrixTranspose(viewProjMatrix));
		DirectX::XMStoreFloat4x4(&mesh2.matrixBufferData.worldMatrix, XMMatrixTranspose(worldMatrix2));


		if (!UpdateMatrixBuffer(immediateContext, mesh1.matrixBuffer, mesh1.matrixBufferData))
		{
			std::cerr << "Failed to update mesh1 matrix buffer!" << std::endl;
			return -1;
		}

		if (!UpdateMatrixBuffer(immediateContext, mesh2.matrixBuffer, mesh2.matrixBufferData))
		{
			std::cerr << "Failed to update mesh2 matrix buffer!" << std::endl;
			return -1;
		}


		if (!UpdateLightingBuffer(immediateContext, mesh1.lightingBuffer, mesh1.lightingBufferData))
		{
			std::cerr << "Failed to update mesh1 lighting buffer!" << std::endl;
			return -1;
		}

		if (!UpdateLightingBuffer(immediateContext, mesh2.lightingBuffer, mesh2.lightingBufferData))
		{
			std::cerr << "Failed to update mesh2 lighting buffer!" << std::endl;
			return -1;
		}

		constexpr float clearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		immediateContext->ClearRenderTargetView(rtv, clearColour);
		immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

		Deprecated_Render(
			immediateContext, rtv,
			dsView, viewport,
			mesh1.vShader, mesh1.pShader,
			mesh1.inputLayout, mesh1.vertexBuffer,
			mesh1.matrixBuffer, mesh1.lightingBuffer,
			mesh1.resourceView, mesh1.samplerState
		);

		Deprecated_Render(
			immediateContext, rtv,
			dsView, viewport,
			mesh2.vShader, mesh2.pShader,
			mesh2.inputLayout, mesh2.vertexBuffer,
			mesh2.matrixBuffer, mesh2.lightingBuffer,
			mesh2.resourceView, mesh2.samplerState
		);

		swapChain->Present(0, 0);
	}

	mesh1.samplerState->Release();
	mesh1.resourceView->Release();
	mesh1.texture2D->Release();
	mesh1.lightingBuffer->Release();
	mesh1.matrixBuffer->Release();
	mesh1.vertexBuffer->Release();
	mesh1.inputLayout->Release();
	mesh1.pShader->Release();
	mesh1.vShader->Release();

	mesh2.samplerState->Release();
	mesh2.resourceView->Release();
	mesh2.texture2D->Release();
	mesh2.lightingBuffer->Release();
	mesh2.matrixBuffer->Release();
	mesh2.vertexBuffer->Release();
	mesh2.inputLayout->Release();
	mesh2.pShader->Release();
	mesh2.vShader->Release();

	dsView->Release();
	dsTexture->Release();
	rtv->Release();
	swapChain->Release();
	immediateContext->Release();
	device->Release();

	return 0;
}





























int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	Deprecated_wWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	return 0;

	constexpr UINT WIDTH = 900;
	constexpr UINT HEIGHT = 900;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	Game game = { };
	if (!game.SetupGraphics(WIDTH, HEIGHT, window))
	{
		std::cerr << "Failed to setup graphics!" << std::endl;
		return -1;
	}

	Scene scene = { };
	if (!game.SetScene(&scene))
	{
		std::cerr << "Failed to set scene!" << std::endl;
		return -1;
	}
	
	Time time = { };
	MSG msg = { };

	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		time.Update();

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!game.Update(time))
		{
			std::cerr << "Failed to update game logic!" << std::endl;
			return -1;
		}

		if (!game.Render(time))
		{
			std::cerr << "Failed to render frame!" << std::endl;
			return -1;
		}
	}

	return 0;
}



































/*
#include "D3D11Helper.h"
#include "PipelineHelper.h"

using namespace DirectX;


void Deprecated_Render(
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


int Deprecated_wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	constexpr UINT WIDTH = 900;
	constexpr UINT HEIGHT = 900;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	Time time = Time();

	ID3D11Device *device;
	ID3D11DeviceContext *immediateContext;
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

	MSG msg{ };
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		time.Update();

		XMVECTOR camPos = { lightingBufferData.camPos[0], lightingBufferData.camPos[1], lightingBufferData.camPos[2] };
		XMVECTOR camDir = XMVector3Normalize(-camPos);

		XMMATRIX viewProjMatrix =
			XMMatrixLookAtLH(camPos, camPos + camDir, { 0, 1, 0 }) *
			XMMatrixPerspectiveFovLH(45.0f * 0.0174533f, (float)WIDTH / HEIGHT, 0.1f, 10.0f);

		XMMATRIX worldMatrix =
			XMMatrixRotationY(-time.time / 1.25f) *
			XMMatrixTranslation(0, 0, 0);

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DirectX::XMStoreFloat4x4(&matrixBufferData.viewProjMatrix, XMMatrixTranspose(viewProjMatrix));
		DirectX::XMStoreFloat4x4(&matrixBufferData.worldMatrix, XMMatrixTranspose(worldMatrix));

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

		Deprecated_Render(
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
*/