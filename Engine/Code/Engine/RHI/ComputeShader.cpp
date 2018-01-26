#include "Engine/RHI/ComputeShader.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

ComputeShader::ComputeShader(RHIDevice* device, const wchar_t* file_path)
{
	ID3DBlob *csBlob = nullptr;
	CompileComputeShader(file_path, "Main", &csBlob);
	
	// Create shader
	dx_computeShader = nullptr;
	HRESULT hr = device->dx_device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &dx_computeShader);

	ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to Create Compute Shader!");

	csBlob->Release();
}

void ComputeShader::CompileComputeShader(const wchar_t* file_path, const char* start_function, ID3DBlob** blob)
{
	LPCSTR entry_point = (LPCSTR)(LPCTSTR)start_function;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif
	
	//const D3D_SHADER_MACRO defines[] =
	//{
	//	"EXAMPLE_DEFINE", "1",
	//	NULL, NULL
	//};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	LPCSTR profile = "cs_5_0";
	HRESULT hr = D3DCompileFromFile(file_path, nullptr/*defines*/, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, profile, flags, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr) || (errorBlob != nullptr)) {
		if (errorBlob != nullptr) {
			char* error_string = (char*)errorBlob->GetBufferPointer();
			DebuggerPrintf("Failed to compile [%s]. Compiler output is: \n%s", file_path, error_string);
			ASSERT_OR_DIE(errorBlob == nullptr, error_string);
			errorBlob->Release();
		}
	}

	*blob = shaderBlob;
}