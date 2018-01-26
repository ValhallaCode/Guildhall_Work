#pragma once
#include "Engine/Render/Vertex.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include <vector>
#include <utility>
#include <set>
#include <map>

class UintVector4;
class Vector4;

struct draw_instruction
{
	unsigned int m_startIndex;
	unsigned int m_vertexCount;
	unsigned int m_primitiveType;
	bool m_usesIndexBuffer;
};

struct physics_idx_set
{
public:
	physics_idx_set(unsigned int idx, bool dirty)
		:is_dirty(dirty)
	{
		idx_set.insert(idx);
	}

public:
	std::set<unsigned int> idx_set;
	bool is_dirty;
};


class MeshBuilder
{
public:
	MeshBuilder();
	~MeshBuilder();
	void GenerateMikkT();
	void Start( const ePrimitiveType& primitive, bool useIndexBuffer);
	void End();
	void Clear();
	void SetTangent(const Vector3& tangent);
	void SetBitangent(const Vector3& bitangent);
	void SetNormal(const Vector3& normal);
	void SetColor(const Rgba& color);
	void SetUV(const Vector2& uv);
	unsigned int AddVertex(const Vector3& position);
	void GenerateIndices();
	void GenerateAdjacency();
	void SetBoneWeightsAndIndices(UintVector4& indices, Vector4& weights);
private:
	Vertex3_PCT m_vertexStamp;
public:
	std::vector<Vertex3_PCT> m_vertices;
	std::vector<unsigned int> m_indices;
	std::map<unsigned int, physics_idx_set> m_adjacency;
	std::vector<draw_instruction> m_instructionList;
	draw_instruction m_currentInstruction;
};