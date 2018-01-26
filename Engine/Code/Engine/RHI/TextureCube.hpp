#pragma  once
#include "Engine/RHI/Image.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/RHI/RHI.hpp"
#include "Engine/RHI/Mesh.hpp"
#include <string>

class TextureCube {
public:
	~TextureCube();
	TextureCube(RHIDevice* device, Image& image);
	TextureCube(RHIDevice* device, std::string filePath);
	void MakeTextureBoxFromImage(RHIDevice* device, const Image& src);

	Texture2D* m_texture;
	Mesh* m_cube;
};