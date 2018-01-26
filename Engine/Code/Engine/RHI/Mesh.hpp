#pragma once
#include "Engine/Render/Vertex.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/Render/MeshBuilder.hpp"
#include <string>
#include <vector>
#include <utility>
#include <set>
#include <map>

class BinaryStream;

class Mesh {
public:
	Mesh(std::string& fileNameAndExtension);
	Mesh(MeshBuilder& builder);
	Mesh();
	~Mesh();
	void LoadMeshFromXML(std::string& fileNameAndExtension);
	void LoadObjMeshFromFile(std::string& fileNameAndExtension);
	void CreateCube(const Vector3& position, const Vector3& extension, const Rgba& color = Rgba(255,255,255,255));
	void CreateTwoSidedQuad(const Vector3& position, const Vector3& extension, const Rgba& color = Rgba(255, 255, 255, 255));
	void CreateTwoSidedQuad(const Vector2& bottomLeft, const Vector2& topRight, const Vector2& uvBottomLeft, const Vector2& uvTopRight, const Rgba& color = Rgba(255, 255, 255, 255));
	void CreateUVSphere(const Vector3& position, unsigned int sliceCount, unsigned int stackCount, const Rgba& color = Rgba(255,255,255,255));
	void CreateInverseCube(const Vector3& position, const Vector3& extension, const Rgba& color = Rgba(255,255,255,255));
	void WriteToStream(BinaryStream* stream);
	void ReadFromStream(BinaryStream* stream);
	void CreateTwoSidedQuadWithBillboard(const Vector3& position, const Vector3& extension, const Vector3& right, const Vector3& up, const Rgba& color = Rgba(255,255,255,255));
	void CreateOneSidedQuad(const Vector3& position, const Vector3& extension, const Rgba& color = Rgba(255, 255, 255, 255));
	void CreateOneSidedQuad(const Vector2& bottomLeft, const Vector2& topRight, const Vector2& uvBottomLeft, const Vector2& uvTopRight, const Rgba& color = Rgba(255, 255, 255, 255));
	void CreateOneSidedQuadWithBillboard(const Vector3& position, const Vector3& extension, const Vector3& right, const Vector3& up, const Rgba& color = Rgba(255,255,255,255));
private:
	void GeneratePhysicsVerts();
	std::vector<uint> GetAdjacentList(uint index_to_check);
	void GetAllOtherOccurencesInIndex(std::vector<uint>& list_to_add, uint val_to_check);
public:
	std::vector<Vertex3_PCT> m_vertices;
	std::vector<PhysicsVert> m_physVerts;
	std::vector<unsigned int> m_indices;
	std::map<unsigned int, physics_idx_set> m_adjacency;
	std::string m_materialLib;
	std::vector<std::string> m_objectsList;
	std::vector<int> m_sides;
	std::vector<std::string> m_materialsList;
	std::string m_type;

	//Fix later
	Vector3 m_extension;
	Rgba m_color;
};