#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/Core/CommandSystem.hpp"
#include "Engine/Render/ParticleSystem.hpp"
#include <map>
#include <string>
#define UNUSED(x) (void)(x);

class Mesh;
class MeshBuilder;
class Skeleton;
class Motion;
class Texture2D;
class Shader;
class Material;
class ShaderProgram;
class KerningFont;
class BitmapFont;
class Renderer;
class CommandSystem;
class SimpleRenderer;
class Profiler;
class ComputeShader;
class TextureDX;
class Texture1D;

//TODO: DEFINE THESE OR YOU WILL BE KILLED!!!
extern unsigned int DEFAULT_WINDOW_WIDTH;
extern unsigned int DEFAULT_WINDOW_HEIGHT;
extern CommandSystem* g_console;
extern Renderer* g_myRenderer;
extern Profiler* g_profiler;
extern ParticleSystem* g_particleSystem;
extern std::map< std::string, Mesh* > g_loadedMeshes;
extern std::map< std::string, Skeleton* > g_loadedSkeletons;
extern std::map< std::string, Motion* > g_loadedMotions;
extern std::map< std::string, TextureDX* > g_loadedTextures;
extern std::map< std::string, Shader* > g_loadedShaders;
extern std::map< std::string, Material* > g_loadedMaterials;
extern std::map< std::string, ShaderProgram* > g_loadedShaderPrograms;
extern std::map< std::string, Sampler* > g_loadedSamplers;
extern std::map< std::string, KerningFont* > g_loadedKerningFonts;
extern std::map< std::string, BitmapFont* > g_loadedBitmapFonts;
extern std::map< std::string, ComputeShader* > g_computeShaderList;

class Input;
extern Input* g_theInputSystem;

class SimpleRenderer;
extern SimpleRenderer* g_simpleRenderer;

Texture2D* CreateOrGetTexture2D(const std::string& name, const SimpleRenderer* renderer = nullptr, const std::string& filePath = "", const std::string& textureType = "default", bool is_uav = false);
Texture1D* CreateOrGetTexture1D(const std::string& name, const SimpleRenderer* renderer = nullptr, const std::string& filePath = "", bool is_uav = false);
ShaderProgram* CreateOrGetShaderProgram(const std::string& name, const std::string& filePath = "", const SimpleRenderer* renderer = nullptr);
Mesh* CreateOrGetMesh(const std::string& meshName, MeshBuilder* builder = nullptr);
Material* CreateOrGetMaterial(const std::string& materialName, SimpleRenderer* renderer = nullptr, const std::string& filePath = "");
Shader* CreateOrGetShader(const std::string& shaderName, tinyxml2::XMLElement* element = nullptr, SimpleRenderer* renderer = nullptr);
Skeleton* CreateOrGetSkeleton(const std::string& skelName);
Motion* CreateOrGetMotion(const std::string& motionName);
Sampler* CreateOrGetSampler(const std::string& samplerName, const SimpleRenderer* renderer = nullptr, eFilterMode minFilter = FILTER_POINT, eFilterMode magFilter = FILTER_POINT, eWrapMode wWrap = WRAPMODE_WRAP, unsigned int anistropic = 1, eWrapMode uWrap = WRAPMODE_WRAP, eWrapMode vWrap = WRAPMODE_WRAP, Rgba border = Rgba(0,0,0,255));
KerningFont* CreateOrGetKerningFont(const std::string& filePath);
BitmapFont* CreateOrGetBitmapFont(const std::string& bitmapFontName, SimpleRenderer* renderer = nullptr);
Mesh* CreateOrGetMesh(const std::string& meshName, const char* file_path);
ComputeShader* CreateOrGetComputeShader(const std::string& name, RHIDevice* device = nullptr, const wchar_t* filepath = nullptr);
void CleanUp();