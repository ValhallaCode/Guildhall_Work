#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/Math/UintVector4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <vector>

struct Vertex2_PCT
{
	Vertex2_PCT()
		:m_position(1.f, 1.f)
		, m_color(255, 255, 255, 255)
		, m_texCoords(1.f, 1.f)
	{
	};
	Vertex2_PCT(const Vector2& position, const Rgba& color, const Vector2& texCoords)
		:m_position(position)
		, m_color(color)
		, m_texCoords(texCoords)
	{
	};

	Vector2 m_position;
	Rgba m_color;
	Vector2 m_texCoords;
};

struct Vertex3_PCT
{
	Vertex3_PCT()
		:m_position(1.f, 1.f, 1.f)
		, m_color(255, 255, 255, 255)
		, m_texCoords(1.f, 1.f)
		, m_normal(0.0f, 0.0f, 0.0f)
		, m_tangent(0.0f, 0.0f, 0.0f)
		, m_bitangent(0.0f, 0.0f, 0.0f)
	{
	};
	Vertex3_PCT(const Vector3& position, const Rgba& color, const Vector2& texCoords)
		:m_position(position)
		, m_color(color)
		, m_texCoords(texCoords)
		, m_normal(0.0f, 0.0f, 0.0f)
		, m_tangent(0.0f,0.0f,0.0f)
		, m_bitangent(0.0f, 0.0f, 0.0f)
	{
	};
	Vertex3_PCT(const Vector3& position, const Rgba& color, const Vector2& texCoords, const Vector3& normal)
		:m_position(position)
		, m_color(color)
		, m_texCoords(texCoords)
		, m_normal(normal)
		, m_tangent(0.0f, 0.0f, 0.0f)
		, m_bitangent(0.0f, 0.0f, 0.0f)
	{
	};
	Vertex3_PCT(const Vector3& position, const Rgba& color, const Vector2& texCoords, const Vector3& normal, const Vector3& tangent, const Vector3& bitangent)
		:m_position(position)
		, m_color(color)
		, m_texCoords(texCoords)
		, m_normal(normal)
		, m_tangent(tangent)
		, m_bitangent(bitangent)
	{
	};
	Vertex3_PCT(const Vertex2_PCT& vertex)
		:m_position(Vector3(vertex.m_position.x, vertex.m_position.y, 0.0f))
		, m_color(vertex.m_color)
		, m_texCoords(vertex.m_texCoords)
		, m_normal(0.0f, 0.0f, 0.0f)
		, m_tangent(0.0f, 0.0f, 0.0f)
		, m_bitangent(0.0f, 0.0f, 0.0f)
	{
	};
	bool operator == (const Vertex3_PCT vertexToEqual) const
	{
		bool posEqual = IsEquivalent(m_position, vertexToEqual.m_position);
		bool texEqual = (m_texCoords == vertexToEqual.m_texCoords);
		bool normEqual = (m_normal == vertexToEqual.m_normal);
		bool tanEqual = (m_tangent == vertexToEqual.m_tangent);
		bool bitanEqual = (m_bitangent == vertexToEqual.m_bitangent);
		bool colorEqual = (m_color == vertexToEqual.m_color);
		return (posEqual && texEqual && normEqual && tanEqual && bitanEqual && colorEqual);
	}

	Vector3 m_position;
	Rgba m_color;
	Vector2 m_texCoords;
	Vector3 m_normal;
	Vector3 m_tangent;
	Vector3 m_bitangent;
	UintVector4 m_boneIndices; 
	Vector4 m_boneWeights;

};

struct Vertex2_PC
{
	Vertex2_PC()
		:m_position(1.f, 1.f)
		, m_color( 255, 255, 255, 255 )
	{
	};
	Vertex2_PC(const Vector2& position, const Rgba& color)
		:m_position(position)
		, m_color(color)
	{
	};

	Vector2 m_position;
	Rgba m_color;
};

struct Vertex3_PC
{
	Vertex3_PC()
		:m_position(1.f, 1.f, 1.f)
		, m_color( 255, 255, 255, 255)
	{
	};
	Vertex3_PC(const Vector3& position, const Rgba& color)
		:m_position(position)
		, m_color(color)
	{
	};

	Vector3 m_position;
	Rgba m_color;
};

struct Vertex_T
{
	Vertex_T() {};
	Vertex_T(const Vector3& position)
		:m_position(position)
	{
	};

	Vector3 m_position;
};

struct PhysicsVert
{
	PhysicsVert() {};
	~PhysicsVert() {};
	Vector3 GetPosition()
	{
		return m_position + m_positionOffset;
	};

	Vector3 m_positionOffset;
	float m_forceToApply;
	float m_forceDampening;
	std::vector<unsigned int> m_neighbors;
	std::vector<float> m_lengthToNeighbor;
	float m_youngsModulous;
	float m_limitOfProportionality;
	float m_limitOfElasticity;
	bool m_isDirty;

	//Simulate
	Vector3 m_position;
	Vector3 m_velocity;
	Vector3 m_force;
};