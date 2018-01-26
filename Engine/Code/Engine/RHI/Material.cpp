#include "Engine/RHI/Material.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/RHI/Shader.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/Rhi/RHI.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/EngineConfig.hpp"
#include <locale>

Material::Material(SimpleRenderer* renderer)
	:m_renderer(renderer)
{
	m_textures.resize(7);

	m_buffers.resize(5);

	m_samplers.resize(3);
}

Material::~Material()
{

}

void Material::set_shader(Shader *shader)
{
	m_shader = shader;
}

void Material::set_sampler(uint const bind, Sampler* sampler)
{
	m_samplers[bind] = sampler;
}

void Material::set_texture(uint const bind, Texture2D* resource)
{
	m_textures[bind] = resource;
}

void Material::set_constant_buffer(uint const bind, ConstantBuffer *buffer)
{
	m_buffers[bind] = buffer;
}

void Material::LoadFromXMLFile(const std::string& filePath)
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile(filePath.c_str());
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	// Load Material Stuff
	m_name = ParseXmlAttribute(*xmlRoot, "name", std::string("MATERIAL_NAME_NOT_FOUND!"));
	
	// Shader
	tinyxml2::XMLElement* shaderElement = xmlRoot->FirstChildElement("Shader");
	std::string shaderName = ParseXmlAttribute(*shaderElement, "name", std::string("SHADER_NAME_NOT_FOUND!"));
	Shader* shader = CreateOrGetShader(shaderName, shaderElement, m_renderer);
	set_shader(shader);

	// Sampler
	for (auto samplerEle = shaderElement->FirstChildElement("Sampler"); samplerEle != nullptr; samplerEle = samplerEle->NextSiblingElement("Sampler"))
	{
		std::string name = ParseXmlAttribute(*samplerEle, "name", std::string("NAME_NOT_FOUND!"));
		unsigned int index = ParseXmlAttribute(*samplerEle, "index", 0);
		std::string minFilterStr = ParseXmlAttribute(*samplerEle, "minFilter", std::string("Filter_Point"));
		eFilterMode minFilter = ConvertStringToFilterMode(minFilterStr);
		std::string magFilterStr = ParseXmlAttribute(*samplerEle, "magFilter", std::string("Filter_Point"));
		eFilterMode magFilter = ConvertStringToFilterMode(magFilterStr);
		std::string wWrapStr = ParseXmlAttribute(*samplerEle, "wWrap", std::string("Wrapmode_Wrap"));
		eWrapMode wWrap = ConvertStringToWrapMode(wWrapStr);
		std::string uWrapStr = ParseXmlAttribute(*samplerEle, "uWrap", std::string("Wrapmode_Wrap"));
		eWrapMode uWrap = ConvertStringToWrapMode(uWrapStr);
		std::string vWrapStr = ParseXmlAttribute(*samplerEle, "vWrap", std::string("Wrapmode_Wrap"));
		eWrapMode vWrap = ConvertStringToWrapMode(vWrapStr);
		unsigned int anisotropic = ParseXmlAttribute(*samplerEle, "anisotropic", 1);
		Rgba border = ParseXmlAttribute(*samplerEle, "border", Rgba(0, 0, 0, 255));

		Sampler* sampler = CreateOrGetSampler(name, m_renderer, minFilter, magFilter, wWrap, anisotropic, uWrap, vWrap, border);
		set_sampler(index, sampler);
	}
	
	// Textures
	tinyxml2::XMLElement* texElement = xmlRoot->FirstChildElement("Textures");
	for (auto texTypeEle = texElement->FirstChildElement(); texTypeEle != nullptr; texTypeEle = texTypeEle->NextSiblingElement())
	{
		std::string texName = ParseXmlAttribute(*texTypeEle, "name", std::string("TEXTURE_NAME_NOT_FOUND!"));
		std::string texType = texTypeEle->Name();
		std::string texFilePath = ParseXmlAttribute(*texTypeEle, "filePath", std::string("TEXTURE_FILE_PATH_NOT_FOUND!"));

		uint typeIndex = GetTextureIndexFromString(texType);
		Texture2D* texture = CreateOrGetTexture2D(texName, m_renderer, texFilePath, texType);
		set_texture(typeIndex, texture);
	}
}

uint Material::GetTextureIndexFromString(const std::string& string)
{
	std::locale local;
	std::string casedType = string;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);

	if (casedType == "diffuse")
	{
		return 0;
	}
	else if (casedType == "normal")
	{
		return 1;
	}
	else if (casedType == "specular")
	{
		return 2;
	}
	else if (casedType == "ambientocclusion")
	{
		return 3;
	}
	else if (casedType == "emmisive")
	{
		return 4;
	}
	else if (casedType == "bump")
	{
		return 5;
	}
	else if (casedType == "height")
	{
		return 6;
	}
	else
	{
		return 0;
	}
}