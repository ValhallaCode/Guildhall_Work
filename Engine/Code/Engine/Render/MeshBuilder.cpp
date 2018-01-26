#include "Engine/Render/MeshBuilder.hpp"
#include <algorithm>

MeshBuilder::MeshBuilder()
{
	Clear();
}

MeshBuilder::~MeshBuilder()
{

}

void MeshBuilder::GenerateMikkT()
{
	// Does nothing currently
}

void MeshBuilder::Start(const ePrimitiveType& primitive, bool useIndexBuffer)
{
	m_currentInstruction.m_primitiveType = primitive;
	m_currentInstruction.m_usesIndexBuffer = useIndexBuffer;
	m_currentInstruction.m_startIndex = m_vertices.size();
}

void MeshBuilder::End()
{

}

void MeshBuilder::Clear()
{
	m_vertexStamp.m_position = Vector3(0.0f, 0.0f, 0.0f);
	m_vertexStamp.m_tangent = Vector3(1.0f, 0.0f, 0.0f);
	m_vertexStamp.m_bitangent = Vector3(0.0f, 1.0f, 0.0f);
	m_vertexStamp.m_normal = Vector3(0.0f, 0.0f, -1.0f);
	m_vertexStamp.m_color = Rgba(255, 255, 255, 255);
	m_vertexStamp.m_texCoords = Vector2(0.0f, 0.0f);
	m_currentInstruction.m_vertexCount = 0;
}

void MeshBuilder::SetTangent(const Vector3& tangent)
{
	m_vertexStamp.m_tangent = tangent;
}

void MeshBuilder::SetBitangent(const Vector3& bitangent)
{
	m_vertexStamp.m_bitangent = bitangent;
}

void MeshBuilder::SetNormal(const Vector3& normal)
{
	m_vertexStamp.m_normal = normal;
}

void MeshBuilder::SetColor(const Rgba& color)
{
	m_vertexStamp.m_color = color;
}

void MeshBuilder::SetUV(const Vector2& uv)
{
	m_vertexStamp.m_texCoords = uv;
}

unsigned int MeshBuilder::AddVertex(const Vector3& position)
{
	m_currentInstruction.m_vertexCount++;
	m_vertexStamp.m_position = position;
	m_vertices.push_back(m_vertexStamp);
	return m_vertices.size() - 1;

}

void MeshBuilder::GenerateIndices()
{
	if (m_vertices.empty())
		return;

	std::vector<Vertex3_PCT> old_verts = m_vertices;
	m_vertices.clear();
	m_vertices.push_back(old_verts[0]);
	m_indices.push_back(0);

	for (uint index = 1; index < old_verts.size(); index++)
	{
		bool was_mapped = false;
		for (uint mapped = 0; mapped < m_vertices.size(); mapped++)
		{
			if (old_verts[index] == m_vertices[mapped])
			{
				was_mapped = true;
				m_indices.push_back(mapped);
				break;
			}
		}

		if (!was_mapped)
		{
			m_indices.push_back(m_vertices.size());
			m_vertices.push_back(old_verts[index]);
		}
	}

	GenerateAdjacency();

	ASSERT_OR_DIE(m_indices.size() == old_verts.size(), "Index Calculation Ended with the wrong amount!");
}

void MeshBuilder::GenerateAdjacency()
{
	for (uint index_to_check = 0; index_to_check < m_vertices.size(); index_to_check++)
	{
		for (uint adjacent_idx = 0; adjacent_idx < m_vertices.size(); adjacent_idx++)
		{
			if(index_to_check == adjacent_idx)
				continue;

			if (IsEquivalent(m_vertices[index_to_check].m_position, m_vertices[adjacent_idx].m_position))
			{
				// Determine Adj
				auto adj_iter = m_adjacency.find(index_to_check);
				if (adj_iter == m_adjacency.end())
				{
					// not there
					physics_idx_set p_idx(adjacent_idx, false);
					m_adjacency.insert_or_assign(index_to_check, p_idx);
				}
				else
				{
					// was there
					adj_iter->second.idx_set.insert(adjacent_idx);
				}
			}
		}
	}
}

void MeshBuilder::SetBoneWeightsAndIndices(UintVector4& indices, Vector4& weights)
{
	m_vertexStamp.m_boneIndices = indices;
	m_vertexStamp.m_boneWeights = weights;
}

