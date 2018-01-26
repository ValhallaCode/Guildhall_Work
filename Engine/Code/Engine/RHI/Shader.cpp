#include "Engine/RHI/Shader.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/EngineConfig.hpp"

Shader::Shader(SimpleRenderer* renderer)
	:m_currentRenderer(renderer)
	, m_name("Default")
	, m_shaderProgram(nullptr)
	, m_raster(nullptr)
	, m_blend(nullptr)
	, m_depthStencil(nullptr)
	, m_depthDesc{}
	, m_depthTestSetting(false)
	, m_depthWriteSetting(false)
	, m_blendStruct(nullptr)
	, m_sourceColor(BLEND_ONE)
	, m_destinationColor(BLEND_ONE)
	, m_wasRasterSet(false)
{

}

Shader::Shader(tinyxml2::XMLElement& element, SimpleRenderer* renderer)
	:m_currentRenderer(renderer)
	, m_blendStruct(nullptr)
{
	m_name = ParseXmlAttribute(element, "name", std::string("NO_SHADER_NAME_FOUND!"));

	tinyxml2::XMLElement* shaderProgramEle = element.FirstChildElement("ShaderProgram");

	std::string shaderProgramName = ParseXmlAttribute(*shaderProgramEle, "name", std::string("Shader_Program_Name_Not_Found!"));
	std::string shaderProgramFilePath = ParseXmlAttribute(*shaderProgramEle, "filePath", std::string("Shader_Program_File_Path_Not_Found!"));

	SetShaderProgram(shaderProgramName, shaderProgramFilePath);

	tinyxml2::XMLElement* rasterEle = element.FirstChildElement("Raster");
	std::string rasterFill = ParseXmlAttribute(*rasterEle, "fillMode", std::string("Raster_Fill_Mode_Not_Found!"));
	std::string rasterCull = ParseXmlAttribute(*rasterEle, "cullMode", std::string("Raster_Cull_Mode_Not_Found!"));
	eFillMode rasterFillMode = GetFillModeFromString(rasterFill);
	eCullMode rasterCullMode = GetCullModeFromString(rasterCull);
	SetRasterState(rasterFillMode, rasterCullMode);

	tinyxml2::XMLElement* blendEle = element.FirstChildElement("Blend");
	bool blendEnable = ParseXmlAttribute(*blendEle, "enable", false);
	std::string blendSourceStr = ParseXmlAttribute(*blendEle, "source", std::string("SOURCE_VALUE_NOT_FOUND!"));
	std::string blendDestStr = ParseXmlAttribute(*blendEle, "destination", std::string("DESTINATION_VALUE_NOT_FOUND!"));
	eBlendFactor source = ConvertStringToBlendFactor(blendSourceStr);
	eBlendFactor dest = ConvertStringToBlendFactor(blendDestStr);
	SetBlendState(blendEnable, source, dest);

	tinyxml2::XMLElement* depthEle = element.FirstChildElement("DepthStencil");
	bool test = ParseXmlAttribute(*depthEle, "test", false);
	bool write = ParseXmlAttribute(*depthEle, "write", false);
	SetDepthStencil(test, write);
}

Shader::~Shader()
{

}

DepthStencilState* Shader::GetDepthStencilState()
{
	return m_depthStencil;
}

BlendState* Shader::GetBlendState()
{
	return m_blend;
}

ShaderProgram* Shader::GetShaderProgram()
{
	return m_shaderProgram;
}

RasterState* Shader::GetRasterState()
{
	return m_raster;
}

void Shader::SetRasterState(const eFillMode& fillMode, const eCullMode& cullMode)
{
	m_raster = new RasterState(m_currentRenderer->m_device, fillMode, cullMode);
}

void Shader::SetBlendState(bool enabled, const eBlendFactor& source, const eBlendFactor& destination)
{
	m_sourceColor = source;
	m_destinationColor = destination;
	m_blend = new BlendState(m_currentRenderer->m_device, enabled, source, destination);
}

void Shader::SetBlendState(BlendState* newBlend)
{
	if (m_blend != nullptr)
	{
		delete m_blend;
		m_blend = nullptr;
	}

	m_blend = newBlend;
}

void Shader::SetShaderProgram(const std::string& name, const std::string& filePath)
{
	m_shaderProgram = CreateOrGetShaderProgram(name, filePath, m_currentRenderer);
}

void Shader::SetDepthStencil(bool enableTest, bool enableWrite)
{
	m_depthTestSetting = enableTest;
	m_depthWriteSetting = enableWrite;
	m_depthDesc.depthTestEnabled = enableTest;
	m_depthDesc.depthWritingEnabled = enableWrite;
	m_depthStencil = new DepthStencilState(m_currentRenderer->m_device, m_depthDesc);
}

void Shader::SetDepthStencil(DepthStencilState* newDepth)
{
	if (m_depthStencil != nullptr)
	{
		delete m_depthStencil;
		m_depthStencil = nullptr;
	}

	m_depthStencil = newDepth;
}

void Shader::EnableDepthTest(bool enable)
{
	m_depthTestSetting = enable;
}

void Shader::EnableDepthWrite(bool enable)
{
	m_depthWriteSetting = enable;
}
