#pragma once
#include "Engine/RHI/DX11.hpp"

class RHIDevice;

class ComputeShader
{
public:
	ComputeShader(RHIDevice* device, const wchar_t* file_path);
	void CompileComputeShader(const wchar_t* file_path, const char* start_function, ID3DBlob** blob);
public:
	ID3D11ComputeShader* dx_computeShader;
};