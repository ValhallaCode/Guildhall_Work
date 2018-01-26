#include "Engine/RHI/RenderMesh.hpp"



RenderMesh::RenderMesh()
	:m_materials{}
	, m_meshs{}
	, m_transform(new Matrix4())
{

}

RenderMesh::~RenderMesh()
{
	m_materials.clear();
	m_meshs.clear();

	delete m_transform;
	m_transform = nullptr;
}

void RenderMesh::UpdateTransform(const Matrix4& matrix)
{
	m_transform->ConcatenateTranform(matrix);
}

void RenderMesh::InsertMesh(const std::string& description, Mesh& mesh)
{
	m_meshs.insert_or_assign(description, mesh);
}

void RenderMesh::InsertMaterial(const std::string& description, Material& material)
{
	m_materials.insert_or_assign(description, material);
}
