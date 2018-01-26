#pragma once
#include "Engine/RHI/Material.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/Math/Matrix4.hpp"
#include <map>
#include <string>

class RenderMesh
{
public:
	RenderMesh();
	~RenderMesh();
	void UpdateTransform(const Matrix4& matrix);
	void InsertMesh(const std::string& description, Mesh& mesh);
	void InsertMaterial(const std::string& description, Material& material);
public:
	std::map<std::string, Material> m_materials;
	std::map<std::string, Mesh> m_meshs;
	Matrix4* m_transform;
};