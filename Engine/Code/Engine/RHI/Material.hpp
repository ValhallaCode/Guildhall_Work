#pragma once
#include <vector>
#include <string>

class SimpleRenderer;
class Texture2D;
class Sampler;
class Shader;
class SimpleRenderer;
class ConstantBuffer;

typedef unsigned int uint;

class Material 
{
public:
	Material(SimpleRenderer* renderer);
	~Material();

public:
	void set_shader(Shader* shader);
	void set_sampler(uint const bind, Sampler* sampler);

	void set_texture(uint const bind, Texture2D *resource);
	void set_constant_buffer(uint const bind, ConstantBuffer *buffer);

	void LoadFromXMLFile(const std::string& filePath);
	uint GetTextureIndexFromString(const std::string& string);
	// If you have a texture database, you can do this by name
	//void set_texture(uint const bind, char const *image_name);

	// common helpers for normal textures
	inline void SetDiffuseMap(Texture2D *tex) { set_texture(0, tex); }
	inline void SetNormalMap(Texture2D *tex) { set_texture(1, tex); }
	inline void SetSpecMap(Texture2D *tex) { set_texture(2, tex); }
	inline void SetAmbientOcclusion(Texture2D *tex) { set_texture(3, tex); }
	inline void SetEmmisive(Texture2D *tex) { set_texture(4, tex); }
	inline void SetBump(Texture2D *tex) { set_texture(5, tex); }
	inline void SetHeight(Texture2D *tex) { set_texture(6, tex); }

public:
	SimpleRenderer* m_renderer;
	Shader* m_shader;
	std::vector<Texture2D*> m_textures;
	std::vector<ConstantBuffer*> m_buffers;
	std::vector<Sampler*> m_samplers;
	std::string m_name;
};