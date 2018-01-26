#pragma once
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/DX11.hpp"

RHIOutput::RHIOutput(RHIDevice* owner, Window* window, IDXGISwapChain* dx_swapChain/*[OPTIONAL]*/ /*, eRHIOutputMode const mode = RENDEROUTPUT_WINDOWED*/)
	:m_device(owner),
	m_window(window),
	m_swapChain(dx_swapChain),
	m_renderTarget(nullptr)
{
	ID3D11Texture2D* backBuffer = nullptr;
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	m_renderTarget = new Texture2D(m_device,backBuffer);
}

RHIOutput::~RHIOutput()
{
	m_swapChain->Release();
	delete m_window;
	delete m_device;
}

void RHIOutput::Present()
{
	m_swapChain->Present(0, 0);
}

void RHIOutput::Close()
{
	m_window->Close();
}

void RHIOutput::Open()
{
	m_window->Open();
}

Texture2D* RHIOutput::GetRenderTarget()
{
	ID3D11Texture2D* dx_texture = nullptr;
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&dx_texture);
	Texture2D* texture = new Texture2D(m_device, dx_texture);
	
	return texture;
}

float RHIOutput::GetWidth() const
{
	IntVector2 dimensions = m_window->GetClientSize();
	return (float)dimensions.x;
}

float RHIOutput::GetHeight() const
{
	IntVector2 dimensions = m_window->GetClientSize();
	return (float)dimensions.y;
}

float RHIOutput::GetAspectRatio() const
{
	IntVector2 dimensions = m_window->GetClientSize();
	return (float)dimensions.x / (float)dimensions.y;
}
