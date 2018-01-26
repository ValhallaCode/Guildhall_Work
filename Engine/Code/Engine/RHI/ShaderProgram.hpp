#pragma once
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"

enum eShaderStage
{
	SHADER_VERTEX,
	SHADER_FRAGMENT,
};

class ShaderProgram
{
public:
	ShaderProgram(RHIDevice* device, ID3D11VertexShader* vs, ID3D11PixelShader* fs, ID3DBlob* vs_BytecodeOut, ID3DBlob* fs_BytecodeOut);
	~ShaderProgram();
	void CreateInputLayout(RHIDevice* device);
	inline bool IsValid() const;

	ID3D11VertexShader* dx_vertexShader;
	ID3D11PixelShader* dx_fragmentShader;
	ID3D11InputLayout* dx_inputLayout;
	ID3DBlob* m_vsBytecode;
	ID3DBlob* m_fsBytecode;
};

//public:
//	ShaderProgram(RHIDevice *device, char const *filename);
//	~ShaderProgram();
//
//	// all shaders must have a vertex stage [outside of compute]
//	inline bool is_valid() const { return (dx_vs != nullptr); }
//
//   public:
//	   // All the steps to this (could be split out to a ShaderStage)
//	   RHIDevice *device;
//
//	   // kept around for convenience for quicker binding
//	   ID3D11VertexShader *dx_vs;
//	   Blob vs_bytecode;
//
//	   ID3D11PixelShader *dx_fs;
//	   Blob fs_bytecode;
