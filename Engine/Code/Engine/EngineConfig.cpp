#include "Engine/EngineConfig.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/Render/Skeleton.hpp"
#include "Engine/Render/Motion.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/RHI/Shader.hpp"
#include "Engine/RHI/Material.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/Render/KerningFont.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Render/MeshBuilder.hpp"
#include "Engine/Core/Profiler.hpp"
#include "Engine/RHI/ComputeShader.hpp"
#include "Engine/RHI/TextureDX.hpp"
#include "Engine/RHI/Texture1D.hpp"
#include <algorithm>
#include <sstream>
#include <locale>

unsigned int DEFAULT_WINDOW_WIDTH = 800;
unsigned int DEFAULT_WINDOW_HEIGHT = 450;
CommandSystem* g_console = nullptr;
Renderer* g_myRenderer = nullptr;
Profiler* g_profiler = nullptr;
ParticleSystem* g_particleSystem = nullptr;
Input* g_theInputSystem = nullptr;
SimpleRenderer* g_simpleRenderer = nullptr;

// Libraries
std::map< std::string, Mesh* > g_loadedMeshes;
std::map< std::string, Skeleton* > g_loadedSkeletons;
std::map< std::string, Motion* > g_loadedMotions;
std::map< std::string, TextureDX* > g_loadedTextures;
std::map< std::string, Shader* > g_loadedShaders;
std::map< std::string, Material* > g_loadedMaterials;
std::map< std::string, ShaderProgram* > g_loadedShaderPrograms;
std::map< std::string, Sampler* > g_loadedSamplers;
std::map< std::string, KerningFont* > g_loadedKerningFonts;
std::map< std::string, BitmapFont* > g_loadedBitmapFonts;
std::map< std::string, ComputeShader* > g_computeShaderList;

Texture2D* CreateOrGetTexture2D(const std::string& name, const SimpleRenderer* renderer /*= nullptr*/, const std::string& filePath /*= ""*/, const std::string& textureType /*= "default"*/, bool is_uav /*= false*/)
{
	//----------------------GET-------------------------------
	for (auto iterate = g_loadedTextures.begin(); iterate != g_loadedTextures.end(); ++iterate)
	{
		TextureDX* dx_tex = iterate->second;
		Texture2D* texture = dynamic_cast<Texture2D*>(dx_tex);
		if (iterate->first == name)
		{
			return texture;
		}
	}

	//---------------------CREATE-----------------------------
	ASSERT_OR_DIE(renderer != nullptr, "Texture " + name + " does not exist and no render object was given!");
	ASSERT_OR_DIE(filePath != "", "Texture " + name + " does not exist and no file path was given!");

	std::stringstream pathStream = std::stringstream(filePath);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(pathStream, segment, '.'))
	{
		seglist.push_back(segment);
	}

	std::locale local;
	std::string casedType = textureType;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);

	if (seglist.size() != 2)
	{
		if (casedType == "diffuse")
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_whiteTexture);
			return renderer->m_whiteTexture;
		}
		else if (casedType == "normal")
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_flatNormalTexture);
			return renderer->m_flatNormalTexture;
		}
		else if (casedType == "specular")
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_whiteTexture);
			return renderer->m_whiteTexture;
		}
		else if (casedType == "occlusion")
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_whiteTexture);
			return renderer->m_whiteTexture;
		}
		else if (casedType == "emmisive")
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_blackTexture);
			return renderer->m_blackTexture;
		}
		else if (casedType == "height")
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_greyTexture);
			return renderer->m_greyTexture;
		}
		else if (casedType == "bump")
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_blackTexture);
			return renderer->m_blackTexture;
		}
		else
		{
			g_loadedTextures.insert_or_assign(name, renderer->m_whiteTexture);
			return renderer->m_whiteTexture;
		}
	}
	else
	{
		Texture2D* texture = new Texture2D(renderer->m_device, filePath, IMAGEFORMAT_RGBA8, is_uav);
		g_loadedTextures.insert_or_assign(name, texture);
		return texture;
	}
}

Texture1D* CreateOrGetTexture1D(const std::string& name, const SimpleRenderer* renderer /*= nullptr*/, const std::string& filePath /*= ""*/, bool is_uav /*= false*/)
{
	//----------------------GET-------------------------------
	for (auto iterate = g_loadedTextures.begin(); iterate != g_loadedTextures.end(); ++iterate)
	{
		TextureDX* dx_tex = iterate->second;
		Texture1D* texture = dynamic_cast<Texture1D*>(dx_tex);
		if (iterate->first == name)
		{
			return texture;
		}
	}

	//---------------------CREATE-----------------------------
	ASSERT_OR_DIE(renderer != nullptr, "Texture " + name + " does not exist and no render object was given!");
	ASSERT_OR_DIE(filePath != "", "Texture " + name + " does not exist and no file path was given!");
	Texture1D* texture = new Texture1D(renderer->m_device, filePath, IMAGEFORMAT_RGBA8, is_uav);
	g_loadedTextures.insert_or_assign(name, texture);
	return texture;
}

ShaderProgram* CreateOrGetShaderProgram(const std::string& name, const std::string& filePath /*= ""*/, const SimpleRenderer* renderer /*= nullptr*/)
{
	for (auto iterate = g_loadedShaderPrograms.begin(); iterate != g_loadedShaderPrograms.end(); ++iterate)
	{
		if (iterate->first == name)
		{
			return iterate->second;
		}
	}

	ASSERT_OR_DIE(renderer != nullptr, "Shader " + name + " does not exist and no render object was given!");
	ASSERT_OR_DIE(filePath != "", "Shader " + name + " does not exist and no file path was given!");

	ShaderProgram* shader = renderer->m_device->CreateShaderFromHLSLFile(filePath);
	g_loadedShaderPrograms.insert_or_assign(name, shader);
	return shader;
}

Mesh* CreateOrGetMesh(const std::string& meshName, MeshBuilder* builder /*= nullptr*/)
{
	for (auto iterate = g_loadedMeshes.begin(); iterate != g_loadedMeshes.end(); ++iterate)
	{
		if (iterate->first == meshName)
		{
			return iterate->second;
		}
	}

	Mesh* mesh = nullptr;
	if (builder != nullptr)
		mesh = new Mesh(*builder);
	else
		mesh = new Mesh();

	g_loadedMeshes.insert_or_assign(meshName, mesh);
	return mesh;
}

Mesh* CreateOrGetMesh(const std::string& meshName, const char* file_path)
{
	for (auto iterate = g_loadedMeshes.begin(); iterate != g_loadedMeshes.end(); ++iterate)
	{
		if (iterate->first == meshName)
		{
			return iterate->second;
		}
	}

	std::string path_string = file_path;
	Mesh* mesh = new Mesh(path_string);

	g_loadedMeshes.insert_or_assign(meshName, mesh);
	return mesh;
}

Material* CreateOrGetMaterial(const std::string& materialName, SimpleRenderer* renderer /*= nullptr*/, const std::string& filePath /*= ""*/)
{
	for (auto iterate = g_loadedMaterials.begin(); iterate != g_loadedMaterials.end(); ++iterate)
	{
		if (iterate->first == materialName)
		{
			return iterate->second;
		}
	}

	ASSERT_OR_DIE(filePath != "", "Material " + materialName + " does not exist and there is no given file path!");
	ASSERT_OR_DIE(renderer != nullptr, "Material " + materialName + " does not exist and no render object was given!");
	Material* material = new Material(renderer);
	material->LoadFromXMLFile(filePath);
	g_loadedMaterials.insert_or_assign(materialName, material);
	return material;
}

Shader* CreateOrGetShader(const std::string& shaderName, tinyxml2::XMLElement* element /*= nullptr*/, SimpleRenderer* renderer /*= nullptr*/)
{
	for (auto iterate = g_loadedShaders.begin(); iterate != g_loadedShaders.end(); ++iterate)
	{
		if (iterate->first == shaderName)
		{
			return iterate->second;
		}
	}

	ASSERT_OR_DIE(renderer != nullptr, "Shader " + shaderName + " does not exist and no render object was given!");
	Shader* shader;
	if (element == nullptr)
		shader = new Shader(renderer);
	else
		shader = new Shader(*element, renderer);
	g_loadedShaders.insert_or_assign(shaderName, shader);
	return shader;
}

Skeleton* CreateOrGetSkeleton(const std::string& skelName)
{
	for (auto iterate = g_loadedSkeletons.begin(); iterate != g_loadedSkeletons.end(); ++iterate)
	{
		if (iterate->first == skelName)
		{
			return iterate->second;
		}
	}

	Skeleton* skeleton = new Skeleton();
	g_loadedSkeletons.insert_or_assign(skelName, skeleton);
	return skeleton;
}

Motion* CreateOrGetMotion(const std::string& motionName)
{
	for (auto iterate = g_loadedMotions.begin(); iterate != g_loadedMotions.end(); ++iterate)
	{
		if (iterate->first == motionName)
		{
			return iterate->second;
		}
	}

	Motion* motion = new Motion();
	g_loadedMotions.insert_or_assign(motionName, motion);
	return motion;
}

Sampler* CreateOrGetSampler(const std::string& samplerName, const SimpleRenderer* renderer /*= nullptr*/, eFilterMode minFilter /*= FILTER_POINT*/, eFilterMode magFilter /*= FILTER_POINT*/, eWrapMode wWrap /*= WRAPMODE_WRAP*/, unsigned int anistropic /*= 1*/, eWrapMode uWrap /*= WRAPMODE_WRAP*/, eWrapMode vWrap /*= WRAPMODE_WRAP*/, Rgba border /*= Rgba(0,0,0,255)*/)
{
	for (auto iterate = g_loadedSamplers.begin(); iterate != g_loadedSamplers.end(); ++iterate)
	{
		if (iterate->first == samplerName)
		{
			return iterate->second;
		}
	}

	ASSERT_OR_DIE(renderer != nullptr, "Sampler " + samplerName + " does not exist and no render object was given!");
	Sampler* shader = new Sampler(renderer->m_device, minFilter, magFilter, uWrap, vWrap, wWrap, anistropic, border);
	g_loadedSamplers.insert_or_assign(samplerName, shader);
	return shader;
}

KerningFont* CreateOrGetKerningFont(const std::string& filePath)
{
	for (auto iterate = g_loadedKerningFonts.begin(); iterate != g_loadedKerningFonts.end(); ++iterate)
	{
		if (iterate->first == filePath)
		{
			return iterate->second;
		}
	}

	KerningFont* font = new KerningFont(filePath);
	g_loadedKerningFonts.insert_or_assign(filePath, font);
	return font;
}

BitmapFont* CreateOrGetBitmapFont(const std::string& bitmapFontName, SimpleRenderer* renderer /*= nullptr*/)
{
	std::map< std::string, BitmapFont* >::iterator found = g_loadedBitmapFonts.find(bitmapFontName);

	if (found == g_loadedBitmapFonts.end())
	{
		ASSERT_OR_DIE(renderer != nullptr, "Bitmap Font " + bitmapFontName + " does not exist and no render object was given!");
		BitmapFont* font = new BitmapFont(bitmapFontName, renderer);
		g_loadedBitmapFonts.insert_or_assign(bitmapFontName, font);
		return font;
	}

	return found->second;
}

ComputeShader* CreateOrGetComputeShader(const std::string& name, RHIDevice* device /*= nullptr*/, const wchar_t* filepath /*= nullptr*/)
{
	std::map< std::string, ComputeShader* >::iterator found = g_computeShaderList.find(name);

	if (found == g_computeShaderList.end())
	{
		ASSERT_OR_DIE(device != nullptr, "Device passed in to create Compute Shader " + name +" is nullptr!");
		ASSERT_OR_DIE(filepath != nullptr, "File Path passed in to create Compute Shader " + name + " is nullptr!");
		ComputeShader* shader = new ComputeShader(device, filepath);
		g_computeShaderList.insert_or_assign(name, shader);
		return shader;
	}

	return found->second;
}

void CleanUp()
{
	g_loadedMeshes.clear();
	g_loadedSkeletons.clear();
	g_loadedMotions.clear();
	g_loadedTextures.clear();
	g_loadedShaders.clear();
	g_loadedMaterials.clear();
	g_loadedShaderPrograms.clear();
	g_loadedSamplers.clear();
	g_loadedKerningFonts.clear();
	g_loadedBitmapFonts.clear();
	g_computeShaderList.clear();
}
