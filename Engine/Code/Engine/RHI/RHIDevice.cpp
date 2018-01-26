#pragma once
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Input/FileUtilities.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"

RHIDevice::RHIDevice()
	:dx_device(nullptr),
	m_immediateContext(nullptr)
{

}

RHIDevice::~RHIDevice()
{
	delete m_immediateContext;
	dx_device->Release();
}

RHIDeviceContext* RHIDevice::GetImmediateContext()
{
	return m_immediateContext;
}

RHIOutput* RHIDevice::CreateOutput(Window* window/*OPTIONAL*/ /*, eRHIOutputMode const mode*/)
{
	if (window == nullptr) {
		return nullptr;
	}
	UINT deviceFlags = 0U;

	IntVector2 windowDimen = IntVector2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	if (window->IsClosed()) {
		window->GetClientSize();
		window->Open();
	}

	DXGI_SWAP_CHAIN_DESC swapDesc;
	memset(&swapDesc, 0, sizeof(swapDesc));												// fill the swap chain description struct
	swapDesc.BufferCount = 2;															// two buffers (one front, one back?)
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;    // how swap chain is to be used
	swapDesc.OutputWindow = window->m_hWnd;												// the window to be copied to on present
	swapDesc.SampleDesc.Count = 1;														// how many multisamples (1 means no multi sampling)
																						// Default options.
	swapDesc.Windowed = TRUE;															// windowed/full-screen mode
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;							// use 32-bit color
	swapDesc.BufferDesc.Width = windowDimen.x;
	swapDesc.BufferDesc.Height = windowDimen.y;

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	IDXGISwapChain* swapChain = nullptr;


	// Actually Create
	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(nullptr, // Adapter, if nullptr, will use adapter window is primarily on.
		D3D_DRIVER_TYPE_HARDWARE,  // Driver Type - We want to use the GPU (HARDWARE)
		nullptr,                   // Software Module - DLL that implements software mode (we do not use)
		deviceFlags,              // device creation options
		nullptr,                   // feature level (use default)
		0U,                        // number of feature levels to attempt
		D3D11_SDK_VERSION,         // SDK Version to use
		&swapDesc,                // Description of our swap chain
		&swapChain,            // Swap Chain we're creating
		&device,               // [out] The device created
		nullptr,                   // [out] Feature Level Acquired
		&deviceContext);            // Context that can issue commands on this pipe.

								 // SUCCEEDED & FAILED are macros provided by Windows to checking
								 // the results.  Almost every D3D call will return one - be sure to check it.
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
	else
	{
		delete window;
		window = nullptr;
	}

	return nullptr;
}

RHIOutput* RHIDevice::CreateOutput(UINT const px_width, UINT const px_height, const IntVector2& position/*OPTIONAL*/ /*, eRHIOutputMode const mode*/)
{
	Window* window = new Window();
	window->SetClientSize(IntVector2(px_width,px_height), position);
	return CreateOutput(window);
}

static ID3DBlob* CompileHLSLToShaderBlob(char const *opt_filename, void const *source_code, size_t const source_code_size, char const *entrypoint, char const* target)
{
	DWORD compile_flags = 0U;

	#if defined(DEBUG_SHADERS)
		compile_flags |= D3DCOMPILE_DEBUG;
		compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
		compile_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;   
	#else 
		compile_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
	#endif

	ID3DBlob* code = nullptr;
	ID3DBlob* errors = nullptr;

	HRESULT hr = ::D3DCompile(source_code, source_code_size, opt_filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint, target, compile_flags, 0, &code, &errors);                         

	if (FAILED(hr) || (errors != nullptr)) {
		if (errors != nullptr) {
			char* error_string = (char*)errors->GetBufferPointer();
			DebuggerPrintf("Failed to compile [%s]. Compiler output is: \n%s", opt_filename, error_string);
			ASSERT_OR_DIE(errors != nullptr, error_string);
			errors->Release();
		}
	}
	return code;
}
//------------------------------------------------------------------------
ShaderProgram* RHIDevice::CreateShaderFromHLSLFile(const std::string& filename)
{
	size_t sourceSize = 0U;
	void* sourceCode = FileReadToBuffer(filename.c_str(), &sourceSize);
	if (sourceCode == nullptr) {
		//ASSERT_OR_DIE( 0, "File not found. %s", filename );
		return nullptr;
	}

	ID3DBlob* vs_Bytecode = nullptr;
	ID3DBlob* fs_Bytecode = nullptr;

	vs_Bytecode = CompileHLSLToShaderBlob(filename.c_str(), sourceCode, sourceSize, "VertexFunction", "vs_5_0");
	fs_Bytecode = CompileHLSLToShaderBlob(filename.c_str(), sourceCode, sourceSize, "FragmentFunction", "ps_5_0");
	::free(sourceCode);

	if ((vs_Bytecode != nullptr)
		&& (fs_Bytecode != nullptr)) {

		ID3D11VertexShader* vs = nullptr;
		ID3D11PixelShader* fs = nullptr;

		dx_device->CreateVertexShader(vs_Bytecode->GetBufferPointer(), vs_Bytecode->GetBufferSize(), nullptr, &vs);

		dx_device->CreatePixelShader(fs_Bytecode->GetBufferPointer(), fs_Bytecode->GetBufferSize(), nullptr, &fs);

		ShaderProgram* program = new ShaderProgram(this, vs, fs, vs_Bytecode, fs_Bytecode);
		return program;
	}

	return nullptr;
}

//------------------------------------------------------------------------
VertexBuffer* RHIDevice::CreateVertexBuffer(Vertex3_PCT* vertices, unsigned int vertex_count, const eBufferUsage& usage)
{
	VertexBuffer* buffer = new VertexBuffer(this, vertices, vertex_count, usage);
	return buffer;
}

IndexBuffer* RHIDevice::CreateIndexBuffer(const std::vector<unsigned int>& vertices, unsigned int vertex_count, const eBufferUsage& usage)
{
	IndexBuffer* buffer = new IndexBuffer(this, vertices, vertex_count, usage);
	return buffer;
}
