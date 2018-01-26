#pragma once
#include "Engine/RHI/RHIInstance.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <initguid.h>
#include <dxgidebug.h>

#include <DXGI.h>
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dxguid.lib" )

RHIInstance* RHIInstance::m_instance = nullptr;

static HMODULE debug_module = nullptr;
static IDXGIDebug* debug = nullptr;

RHIInstance::~RHIInstance()
{
	ReportLiveObjects();
}

RHIInstance* RHIInstance::GetInstance() {
	if (m_instance) {
		return m_instance;
	} else {
		m_instance = new RHIInstance();
		return m_instance;
	}
}

RHIInstance::RHIInstance() 
{

}

RHIOutput* RHIInstance::CreateOutput(UINT const px_width, UINT const px_height, const IntVector2& position/*OPTIONAL*/ /*, eRHIOutputMode const initial_mode = RENDEROUTPUT_WINDOWED*/)
{
	UINT deviceFlags = 0U;
	#if defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	Window* window = new Window();
	window->SetClientSize(IntVector2(px_width,px_height), position);
	window->Open();

	DXGI_SWAP_CHAIN_DESC swapDesc;
	memset(&swapDesc, 0, sizeof(swapDesc));												// fill the swap chain description struct
	swapDesc.BufferCount = 2;															// two buffers (one front, one back?)
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;    // how swap chain is to be used
	swapDesc.OutputWindow = window->m_hWnd;												// the window to be copied to on present
	swapDesc.SampleDesc.Count = 1;														// how many multisamples (1 means no multi sampling)
																						// Default options.
	swapDesc.Windowed = TRUE;															// windowed/full-screen mode
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;							// use 32-bit color
	swapDesc.BufferDesc.Width = px_width;
	swapDesc.BufferDesc.Height = px_height;

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	IDXGISwapChain* swapChain = nullptr;

	
	// Actually Create
	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(nullptr,	// Adapter, if nullptr, will use adapter window is primarily on.
		D3D_DRIVER_TYPE_HARDWARE,							// Driver Type - We want to use the GPU (HARDWARE)
		nullptr,											// Software Module - DLL that implements software mode (we do not use)
		deviceFlags,										// device creation options
		nullptr,											// feature level (use default)
		0U,													// number of feature levels to attempt
		D3D11_SDK_VERSION,									// SDK Version to use
		&swapDesc,											// Description of our swap chain
		&swapChain,											// Swap Chain we're creating
		&device,											// [out] The device created
		nullptr,											// [out] Feature Level Acquired
		&deviceContext);									// Context that can issue commands on this pipe.

	// SUCCEEDED & FAILED are macros provided by Windows to checking the results.  Almost every D3D call will return one - be sure to check it.

	#if defined(_DEBUG)
		// Debug Setup
		debug_module = ::LoadLibraryA("Dxgidebug.dll");
		typedef HRESULT(WINAPI *GetDebugModuleCB)(REFIID, void**);
		GetDebugModuleCB cb = (GetDebugModuleCB) ::GetProcAddress(debug_module, "DXGIGetDebugInterface");
		HRESULT result = cb(__uuidof(IDXGIDebug), (void**)&debug);
		ASSERT_RECOVERABLE(SUCCEEDED(result), "Failed to make DEBUG state for RHIInstance!");
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	#endif

	bool success = SUCCEEDED(hr);
	if (success)
	{
		RHIDevice* rhi_device = new RHIDevice();
		rhi_device->dx_device = device;
		RHIDeviceContext* rhi_deviceContext = new RHIDeviceContext(rhi_device, deviceContext);
		rhi_device->m_immediateContext = rhi_deviceContext;

		RHIOutput* rhi_output = new RHIOutput(rhi_device, window, swapChain);
		return rhi_output;
	}
	else {
		delete window;
		window = nullptr;
	}
	return nullptr;
}

void RHIInstance::SetDebugName(ID3D11DeviceChild *d3d_obj, char const *name)
{
	#if defined(_DEBUG)
		if ((d3d_obj != nullptr) && (name != nullptr)) 
		{
			d3d_obj->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name) + 1, name);
		}
	#endif
}

void RHIInstance::ReportLiveObjects()
{
	if (m_instance != nullptr)
	{
		if (debug != nullptr)
		{
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
		}
	}
}