#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/Render/Vertex.hpp"

ShaderProgram::ShaderProgram(RHIDevice* device, ID3D11VertexShader* vs, ID3D11PixelShader* fs, ID3DBlob* vsBytecodeOut, ID3DBlob* fsBytecodeOut)
	: dx_vertexShader(vs)
	, dx_fragmentShader(fs)
	, m_vsBytecode(vsBytecodeOut)
	, m_fsBytecode(fsBytecodeOut)
{
	dx_inputLayout = nullptr;
	CreateInputLayout(device);
}

ShaderProgram::~ShaderProgram()
{
	dx_vertexShader->Release();
	dx_fragmentShader->Release();
	dx_inputLayout->Release();
	m_vsBytecode->Release();
	m_fsBytecode->Release();
}

void ShaderProgram::CreateInputLayout(RHIDevice* device)
{
	D3D11_INPUT_ELEMENT_DESC desc[8];
	memset(desc, 0, sizeof(desc));
	desc[0].SemanticName = "POSITION";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[0].InputSlot = 0U;
	desc[0].AlignedByteOffset = offsetof(Vertex3_PCT, m_position);
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0U;

	desc[1].SemanticName = "UV";
	desc[1].SemanticIndex = 0;
	desc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	desc[1].InputSlot = 0U;
	desc[1].AlignedByteOffset = offsetof(Vertex3_PCT, m_texCoords);
	desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[1].InstanceDataStepRate = 0U;

	desc[2].SemanticName = "COLOR";
	desc[2].SemanticIndex = 0;
	desc[2].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc[2].InputSlot = 0U;
	desc[2].AlignedByteOffset = offsetof(Vertex3_PCT, m_color);
	desc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[2].InstanceDataStepRate = 0U;

	desc[3].SemanticName = "NORMAL";
	desc[3].SemanticIndex = 0;
	desc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[3].InputSlot = 0U;
	desc[3].AlignedByteOffset = offsetof(Vertex3_PCT, m_normal);
	desc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[3].InstanceDataStepRate = 0U;

	desc[4].SemanticName = "TANGENT";
	desc[4].SemanticIndex = 0;
	desc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[4].InputSlot = 0U;
	desc[4].AlignedByteOffset = offsetof(Vertex3_PCT, m_tangent);
	desc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[4].InstanceDataStepRate = 0U;

	desc[5].SemanticName = "BITANGENT";
	desc[5].SemanticIndex = 0;
	desc[5].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[5].InputSlot = 0U;
	desc[5].AlignedByteOffset = offsetof(Vertex3_PCT, m_bitangent);
	desc[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[5].InstanceDataStepRate = 0U;

	desc[6].SemanticName = "BONE_INDICES";
	desc[6].SemanticIndex = 0;
	desc[6].Format = DXGI_FORMAT_R32G32B32A32_UINT;
	desc[6].InputSlot = 0U;
	desc[6].AlignedByteOffset = offsetof(Vertex3_PCT, m_boneIndices);
	desc[6].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[6].InstanceDataStepRate = 0U;

	desc[7].SemanticName = "BONE_WEIGHTS";
	desc[7].SemanticIndex = 0;
	desc[7].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc[7].InputSlot = 0U;
	desc[7].AlignedByteOffset = offsetof(Vertex3_PCT, m_boneWeights);
	desc[7].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[7].InstanceDataStepRate = 0U;


	device->dx_device->CreateInputLayout(desc, ARRAYSIZE(desc), m_vsBytecode->GetBufferPointer(), m_vsBytecode->GetBufferSize(), &dx_inputLayout);
}

bool ShaderProgram::IsValid() const
{
	return (dx_vertexShader != nullptr) && (dx_fragmentShader != nullptr);
}
