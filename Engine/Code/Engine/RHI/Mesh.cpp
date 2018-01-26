#include "Engine/RHI/Mesh.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Math3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/BinaryStream.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include <sstream>
#include <fstream>
#include <iostream>



Mesh::Mesh(std::string& fileNameAndExtension)
{
	std::stringstream pathStream = std::stringstream(fileNameAndExtension);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(pathStream, segment, '.'))
	{
		seglist.push_back(segment);
	}

	std::string fileType = seglist[1];

	if (fileType == "obj")
	{
		LoadObjMeshFromFile(fileNameAndExtension);
	}
	else if (fileType == "xml")
	{
		LoadMeshFromXML(fileNameAndExtension);
	}
	else
	{
		ASSERT_OR_DIE(true, "Unsupported File Type Used to Load Mesh!");
	}
}

Mesh::Mesh() 
{

};

Mesh::Mesh(MeshBuilder& builder)
{
	m_vertices = builder.m_vertices;
	m_indices = builder.m_indices;
	m_adjacency = builder.m_adjacency;
	GeneratePhysicsVerts();
}

Mesh::~Mesh()
{

}

void Mesh::LoadMeshFromXML(std::string& fileNameAndExtension)
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile(fileNameAndExtension.c_str());
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	auto mesh_type_xml = xmlRoot->FirstChildElement();
	if (mesh_type_xml == nullptr)
	{
		return;
	}

	std::string mesh_type = mesh_type_xml->Name();
	m_type = mesh_type;
	if (mesh_type == "One_Sided_Quad")
	{
		auto posElem = mesh_type_xml->FirstChildElement("Position");
		Vector3 position = ParseXmlAttribute(*posElem, "val", Vector3(0.0f, 0.0f, 0.0f));
		auto extElem = mesh_type_xml->FirstChildElement("Extensions");
		m_extension = ParseXmlAttribute(*extElem, "val", Vector3(1.0f, 1.0f, 0.0f));
		auto colorElem = mesh_type_xml->FirstChildElement("Tint");
		m_color = ParseXmlAttribute(*colorElem, "val", Rgba(255, 255, 255, 255));

		CreateOneSidedQuad(position, m_extension, m_color);
	}
	else if (mesh_type == "Two_Sided_Quad")
	{
		auto posElem = mesh_type_xml->FirstChildElement("Position");
		Vector3 position = ParseXmlAttribute(*posElem, "val", Vector3(0.0f, 0.0f, 0.0f));
		auto extElem = mesh_type_xml->FirstChildElement("Extensions");
		m_extension = ParseXmlAttribute(*extElem, "val", Vector3(1.0f, 1.0f, 0.0f));
		auto colorElem = mesh_type_xml->FirstChildElement("Tint");
		m_color = ParseXmlAttribute(*colorElem, "val", Rgba(255, 255, 255, 255));

		CreateTwoSidedQuad(position, m_extension, m_color);
	}
	else if (mesh_type == "Cube")
	{
		auto posElem = mesh_type_xml->FirstChildElement("Position");
		Vector3 position = ParseXmlAttribute(*posElem, "val", Vector3(0.0f, 0.0f, 0.0f));
		auto extElem = mesh_type_xml->FirstChildElement("Extensions");
		Vector3 extension = ParseXmlAttribute(*extElem, "val", Vector3(1.0f, 1.0f, 1.0f));
		auto colorElem = mesh_type_xml->FirstChildElement("Tint");
		Rgba color = ParseXmlAttribute(*colorElem, "val", Rgba(255, 255, 255, 255));

		CreateCube(position, extension, color);
	}
	else if (mesh_type == "UV_Sphere")
	{
		auto posElem = mesh_type_xml->FirstChildElement("Position");
		Vector3 position = ParseXmlAttribute(*posElem, "val", Vector3(0.0f, 0.0f, 0.0f));
		auto sliceElem = mesh_type_xml->FirstChildElement("Slices");
		unsigned int slices = ParseXmlAttribute(*sliceElem, "val", 4);
		auto stackElem = mesh_type_xml->FirstChildElement("Stacks");
		unsigned int stacks = ParseXmlAttribute(*stackElem, "val", 2);
		auto colorElem = mesh_type_xml->FirstChildElement("Tint");
		Rgba color = ParseXmlAttribute(*colorElem, "val", Rgba(255, 255, 255, 255));

		CreateUVSphere(position, slices, stacks, color);
	}
	else if (mesh_type == "Inverse_Cube")
	{
		auto posElem = mesh_type_xml->FirstChildElement("Position");
		Vector3 position = ParseXmlAttribute(*posElem, "val", Vector3(0.0f, 0.0f, 0.0f));
		auto extElem = mesh_type_xml->FirstChildElement("Extensions");
		Vector3 extension = ParseXmlAttribute(*extElem, "val", Vector3(1.0f, 1.0f, 1.0f));
		auto colorElem = mesh_type_xml->FirstChildElement("Tint");
		Rgba color = ParseXmlAttribute(*colorElem, "val", Rgba(255, 255, 255, 255));

		CreateInverseCube(position, extension, color);
	}
	else
	{
		ASSERT_OR_DIE(false, "Mesh Type [" + mesh_type +"] is not supported!");
	}
}

void Mesh::LoadObjMeshFromFile(std::string& fileNameAndExtension)
{
	std::ifstream ss("Data/Mesh/OBJ/" + fileNameAndExtension);
	std::vector<Vector3> Position;
	std::vector<Vector2> TexCoord;
	std::vector<Vector3> Normal;

	char cmd[256] = { 0 };
	while (true)
	{
		ss >> cmd;
		if (!ss)
			break;

		if (0 == strcmp(cmd, "#"))
		{
			continue;
		}
		else if (0 == strcmp(cmd, "g"))
		{
			std::string objectName;
			ss >> objectName;
			m_objectsList.push_back(objectName);
		}
		else if (0 == strcmp(cmd, "s"))
		{
			int value;
			ss >> value;
			m_sides.push_back(value);
		}
		else if (0 == strcmp(cmd, "usemtl"))
		{
			std::string materialName;
			ss >> materialName;
			m_materialsList.push_back(materialName);
		}
		else if (0 == strcmp(cmd, "mtllib"))
		{
			ss >> m_materialLib;
		}
		else if (0 == strcmp(cmd, "v"))
		{
			float x, y, z;
			ss >> x >> y >> z;
			Position.push_back(Vector3(x, y, z));
		}
		else if (0 == strcmp(cmd, "vt"))
		{
			float u, v, w;
			ss >> u >> v >> w;
			TexCoord.push_back(Vector2(u, 1.0f-v));
		}
		else if (0 == strcmp(cmd, "vn"))
		{
			float x, y, z;
			ss >> x >> y >> z;
			Normal.push_back(Vector3(x, y, z));
		}
		else if (0 == strcmp(cmd, "f"))
		{
			unsigned int Value; 
			Vertex3_PCT vertex;
			std::string line;
			std::getline(ss, line);
			std::istringstream iss(line);
			unsigned int count = 0;
			unsigned int startIndex = static_cast<unsigned int>(m_vertices.size());

			while(iss >> Value)
			{
				vertex.m_position = Position[Value - 1];
				iss.ignore();

				iss >> Value;
				vertex.m_texCoords = TexCoord[Value - 1];
				iss.ignore();

				iss >> Value;
				vertex.m_normal = Normal[Value - 1];

				++count;
				m_vertices.push_back(vertex);
			}

			if (count == 4)
			{
				m_indices.push_back(startIndex);
				m_indices.push_back(startIndex + 2);
				m_indices.push_back(startIndex + 1);
				m_indices.push_back(startIndex);
				m_indices.push_back(startIndex + 3);
				m_indices.push_back(startIndex + 2);
			}
			else
			{
				m_indices.push_back(startIndex);
				m_indices.push_back(startIndex + 2);
				m_indices.push_back(startIndex + 1);
			}
		}
	}

	m_vertices.shrink_to_fit();
	m_indices.shrink_to_fit();
}

void Mesh::CreateOneSidedQuad(const Vector3& position, const Vector3& extension, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	m_vertices = {
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f)),/*0*/
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f)),/*1*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f)),/*2*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f))/*3*/
	};

	m_indices = { 0,3,2,0,2,1 };
}

void Mesh::CreateOneSidedQuadWithBillboard(const Vector3& position, const Vector3& extension, const Vector3& right, const Vector3& up, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	Vector3 bottom_left = position + -extension.x * right + -extension.y * up;
	Vector3 top_left = position + -extension.x * right + extension.y * up;
	Vector3 bottom_right = position + extension.x * right + -extension.y * up;
	Vector3 top_right = position + extension.x * right + extension.y * up;

	Vector3 normal = CrossProduct3D(up, right);

	m_vertices = {
		Vertex3_PCT(bottom_left, color, Vector2(0.0f,1.0f), normal),/*0*/
		Vertex3_PCT(top_left, color, Vector2(0.0f,0.0f), normal),/*1*/
		Vertex3_PCT(top_right, color, Vector2(1.0f,0.0f), normal),/*2*/
		Vertex3_PCT(bottom_right, color, Vector2(1.0f,1.0f), normal)/*3*/
	};

	m_indices = { 0,3,2,0,2,1 };
}

void Mesh::GeneratePhysicsVerts()
{
	m_physVerts.resize(m_vertices.size());


	for (uint vert_index = 0; vert_index < m_vertices.size(); vert_index++)
	{
		std::vector<uint> adj_list = GetAdjacentList(vert_index);
		m_physVerts[vert_index].m_neighbors = adj_list;
		m_physVerts[vert_index].m_lengthToNeighbor.resize(adj_list.size());

		Vector3 my_position = m_vertices[vert_index].m_position;

		for (uint adj_index = 0; adj_index < adj_list.size(); adj_index++)
		{
			Vector3 adj_pos = m_vertices[adj_list[adj_index]].m_position;
			Vector3 displacement = adj_pos - my_position;
			m_physVerts[vert_index].m_lengthToNeighbor[adj_index] = displacement.CalcLength();
		}

		m_physVerts[vert_index].m_youngsModulous = 0.0f;
		m_physVerts[vert_index].m_limitOfProportionality = 0.0f;
		m_physVerts[vert_index].m_limitOfElasticity = 0.0f;
		m_physVerts[vert_index].m_forceDampening = 0.0f;
		m_physVerts[vert_index].m_forceToApply = 0.0f;
		m_physVerts[vert_index].m_isDirty = false;
		m_physVerts[vert_index].m_position = m_vertices[vert_index].m_position;
	}
}

std::vector<uint> Mesh::GetAdjacentList(uint index_to_check)
{
	std::vector<uint> adjacent_list;

	auto adj_iter = m_adjacency.find(index_to_check);
	if (adj_iter != m_adjacency.end())
	{
		for (uint adj_idx : adj_iter->second.idx_set)
		{
			GetAllOtherOccurencesInIndex(adjacent_list, adj_idx);
		}
	}

	GetAllOtherOccurencesInIndex(adjacent_list, index_to_check);

	return adjacent_list;
}

void Mesh::GetAllOtherOccurencesInIndex(std::vector<uint>& list_to_add, uint val_to_check)
{
	// Find each occurence in master list
	auto idx_iter = m_indices.begin();
	while ((idx_iter = std::find(idx_iter, m_indices.end(), val_to_check)) != m_indices.end())
	{
		//Get Index in master list
		uint idx_in_master = idx_iter - m_indices.begin();

		//Determine Where in its traingle it is
		uint mod_for_tri_order = idx_in_master % 3;

		//Grab indexes to change accordingly
		if (mod_for_tri_order == 0)
		{
			uint val1_adj = m_indices[idx_in_master + 1];
			uint val2_adj = m_indices[idx_in_master + 2];

			if(std::find(list_to_add.begin(), list_to_add.end(), val1_adj) == list_to_add.end())
				list_to_add.push_back(val1_adj);
			if (std::find(list_to_add.begin(), list_to_add.end(), val2_adj) == list_to_add.end())
				list_to_add.push_back(val2_adj);
		}
		else if (mod_for_tri_order == 1)
		{
			uint val1_adj = m_indices[idx_in_master - 1];
			uint val2_adj = m_indices[idx_in_master + 1];

			if (std::find(list_to_add.begin(), list_to_add.end(), val1_adj) == list_to_add.end())
				list_to_add.push_back(val1_adj);
			if (std::find(list_to_add.begin(), list_to_add.end(), val2_adj) == list_to_add.end())
				list_to_add.push_back(val2_adj);
		}
		else
		{
			uint val1_adj = m_indices[idx_in_master - 2];
			uint val2_adj = m_indices[idx_in_master - 1];

			if (std::find(list_to_add.begin(), list_to_add.end(), val1_adj) == list_to_add.end())
				list_to_add.push_back(val1_adj);
			if (std::find(list_to_add.begin(), list_to_add.end(), val2_adj) == list_to_add.end())
				list_to_add.push_back(val2_adj);
		}

		idx_iter++;
	}
}

void Mesh::CreateTwoSidedQuadWithBillboard(const Vector3& position, const Vector3& extension, const Vector3& right, const Vector3& up, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	Vector3 bottom_left = position + -extension.x * right + -extension.y * up;
	Vector3 top_left = position + -extension.x * right + extension.y * up;
	Vector3 bottom_right = position + extension.x * right + -extension.y * up;
	Vector3 top_right = position + extension.x * right + extension.y * up;

	Vector3 normal = CrossProduct3D(up, right);

	m_vertices = {
		Vertex3_PCT(bottom_left, color, Vector2(0.0f,1.0f), normal),/*0*/
		Vertex3_PCT(top_left, color, Vector2(0.0f,0.0f), normal),/*1*/
		Vertex3_PCT(top_right, color, Vector2(1.0f,0.0f), normal),/*2*/
		Vertex3_PCT(bottom_right, color, Vector2(1.0f,1.0f), normal)/*3*/
	};

	m_indices = { 0,3,2,0,2,1,  3,0,1,3,1,2 };
}

void Mesh::CreateOneSidedQuad(const Vector2& bottomLeft, const Vector2& topRight, const Vector2& uvBottomLeft, const Vector2& uvTopRight, const Rgba& color /*= Rgba(255, 255, 255, 255)*/)
{
	Vector2 topLeft(bottomLeft.x, topRight.y);
	Vector2 bottomRight(topRight.x, bottomLeft.y);

	m_vertices = {
		Vertex3_PCT(Vector3(bottomLeft.x, bottomLeft.y, 0.0f), color, uvBottomLeft),							/*0*/
		Vertex3_PCT(Vector3(topLeft.x, topLeft.y, 0.0f), color, Vector2(uvBottomLeft.x,uvTopRight.y)),		/*1*/
		Vertex3_PCT(Vector3(topRight.x, topRight.y, 0.0f), color, uvTopRight),									/*2*/
		Vertex3_PCT(Vector3(bottomRight.x, bottomRight.y, 0.0f), color, Vector2(uvTopRight.x,uvBottomLeft.y))	/*3*/
	};

	m_indices = { 0,3,2,0,2,1 };
}

void Mesh::CreateCube(const Vector3& position, const Vector3& extension, const Rgba& color /*= Rgba(255, 255, 255, 255)*/)
{
	Vector3 LowerLeftFront = position + Vector3(-extension.x, -extension.y, -extension.z);
	Vector3 UpperLeftFront = position + Vector3(-extension.x, extension.y, -extension.z);
	Vector3 LowerRightFront = position + Vector3(extension.x, -extension.y, -extension.z);
	Vector3 UpperRightFront = position + Vector3(extension.x, extension.y, -extension.z);

	Vector3 LowerLeftBack = position + Vector3(-extension.x, -extension.y, extension.z);
	Vector3 UpperLeftBack = position + Vector3(-extension.x, extension.y, extension.z);
	Vector3 LowerRightBack = position + Vector3(extension.x, -extension.y, extension.z);
	Vector3 UpperRightBack = position + Vector3(extension.x, extension.y, extension.z);

	Vector3 BottomFrontNorm = LowerRightFront - LowerLeftFront;
	BottomFrontNorm.Normalize();
	Vector3 LeftFrontNorm = UpperLeftFront - LowerLeftFront;
	LeftFrontNorm.Normalize();
	Vector3 RightFrontNorm = UpperRightFront - LowerRightFront;
	RightFrontNorm.Normalize();
	Vector3 TopFrontNorm = UpperLeftFront - UpperRightFront;
	TopFrontNorm.Normalize();

	Vector3 BottomBackNorm = LowerRightBack - LowerLeftBack;
	BottomBackNorm.Normalize();
	Vector3 LeftBackNorm = UpperLeftBack - LowerLeftBack;
	LeftBackNorm.Normalize();
	Vector3 RightBackNorm = UpperRightBack - LowerRightBack;
	RightBackNorm.Normalize();
	Vector3 TopBackNorm = UpperLeftBack - UpperRightBack;
	TopBackNorm.Normalize();

	Vector3 RightBottomNorm = LowerRightBack - LowerRightFront;
	RightBottomNorm.Normalize();
	Vector3 RightTopNorm = UpperRightBack - UpperRightFront;
	RightTopNorm.Normalize();

	Vector3 LeftBottomNorm = LowerLeftBack - LowerLeftFront;
	LeftBottomNorm.Normalize();
	Vector3 LeftTopNorm = UpperLeftBack - UpperLeftFront;
	LeftTopNorm.Normalize();

	Vector3 FrontFaceNormal = CrossProduct3D(BottomFrontNorm, LeftFrontNorm);
	Vector3 BackFaceNormal = -1.0f * FrontFaceNormal;
	Vector3 RightFaceNormal = CrossProduct3D(RightBottomNorm, RightFrontNorm);
	Vector3 LeftFaceNorm = -1.0f * RightFaceNormal;
	Vector3 BottomFaceNorm = CrossProduct3D(BottomFrontNorm, LeftBottomNorm);
	Vector3 TopFaceNorm = -1.0f * BottomFaceNorm;

	m_vertices = {
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, -extension.z), color, Vector2(0.0f,1.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*0*/ //Back Face
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, -extension.z), color, Vector2(0.0f,0.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*1*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, -extension.z), color, Vector2(1.0f,0.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*2*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, -extension.z), color, Vector2(1.0f,1.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*3*/

		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*4*/ //Front Face
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*5*/
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*6*/
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*7*/

		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*8*/ //Left Face
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*9*/
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, -extension.z), color, Vector2(1.0f,0.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*10*/
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, -extension.z), color, Vector2(1.0f,1.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*11*/

		Vertex3_PCT(position + Vector3(extension.x, -extension.y, -extension.z), color, Vector2(0.0f,1.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*12*/ //Right Face
		Vertex3_PCT(position + Vector3(extension.x, extension.y, -extension.z), color, Vector2(0.0f,0.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*13*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*14*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*15*/

		Vertex3_PCT(position + Vector3(-extension.x, extension.y, -extension.z), color, Vector2(0.0f,1.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*16*/ //Top Face
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*17*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*18*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, -extension.z), color, Vector2(1.0f,1.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*19*/

		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*20*/ //Bottom Face
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, -extension.z), color, Vector2(0.0f,0.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*21*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, -extension.z), color, Vector2(1.0f,0.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*22*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*23*/
	};

	m_indices = { 0,3,2,0,2,1, 4,7,6,4,6,5, 8,11,10,8,10,9, 12,15,14,12,14,13, 16,19,18,16,18,17, 20,23,22,20,22,21 };
}

void Mesh::CreateTwoSidedQuad(const Vector3& position, const Vector3& extension, const Rgba& color /*= Rgba(255, 255, 255, 255)*/)
{
	m_vertices = {
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f)),/*0*/
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f)),	/*1*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f)),	/*2*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f))	/*3*/
	};

	m_indices = { 0,3,2,0,2,1,  3,0,1,3,1,2 };
}

void Mesh::CreateTwoSidedQuad(const Vector2& bottomLeft, const Vector2& topRight, const Vector2& uvBottomLeft, const Vector2& uvTopRight, const Rgba& color /*= Rgba(255, 255, 255, 255)*/)
{
	Vector2 topLeft(bottomLeft.x, topRight.y);
	Vector2 bottomRight(topRight.x, bottomLeft.y);

	m_vertices = {
		Vertex3_PCT(Vector3(bottomLeft.x, bottomLeft.y, 0.0f), color, uvBottomLeft),							/*0*/
		Vertex3_PCT(Vector3(topLeft.x, topLeft.y, 0.0f), color, Vector2(uvBottomLeft.x,uvTopRight.y)),			/*1*/
		Vertex3_PCT(Vector3(topRight.x, topRight.y, 0.0f), color, uvTopRight),									/*2*/
		Vertex3_PCT(Vector3(bottomRight.x, bottomRight.y, 0.0f), color, Vector2(uvTopRight.x,uvBottomLeft.y))	/*3*/
	};

	m_indices = { 0,3,2,0,2,1,  3,0,1,3,1,2 };
}

void Mesh::CreateUVSphere(const Vector3& position, unsigned int sliceCount, unsigned int stackCount, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	float phiStep = PI / (float)stackCount;
	float thetaStep = 2.0f * (PI / (float)sliceCount);
	
	for (unsigned int phiIndex = 0; phiIndex <= stackCount; ++phiIndex)
	{
		float phi = phiIndex  * phiStep;
		for (unsigned int thetaIndex = 0; thetaIndex <= sliceCount; ++thetaIndex)
		{
			float theta = thetaIndex * thetaStep;
			Vector3 point = Vector3(sinf(phi) * cosf(theta), cosf(phi), sinf(phi) * sinf(theta)) + position;
			Vector3 tangent(-sinf(phi) * sinf(theta), 0, sinf(phi) * cosf(theta));
			Vector3 normal = point - position;
			normal.Normalize();
			tangent.Normalize();
			Vector3 bitangent = CrossProduct3D(normal, tangent);
			bitangent.Normalize();
			Vector2 uv(theta / (PI * 2.0f), phi / PI);

			m_vertices.push_back(Vertex3_PCT(point, color, uv, normal, tangent, bitangent));
		}
	}

	unsigned int ringVertexCount = sliceCount;
	for (unsigned int index = 0; index < sliceCount; ++index)
	{
		m_indices.push_back(index);
		m_indices.push_back(index + (ringVertexCount + 1));
		m_indices.push_back(index + (ringVertexCount + 2));
	}

	unsigned int baseIndex = 1;
	for (unsigned int index = 1; index < stackCount - 1; ++index)
	{
		for (unsigned int step = 0; step <= sliceCount; ++step)
		{
			m_indices.push_back(baseIndex + ((index - 1) * ringVertexCount) + step);
			m_indices.push_back(baseIndex + ((index - 1) * ringVertexCount) + (step - 1));
			m_indices.push_back(baseIndex + (index * (ringVertexCount - 1)) + (step + baseIndex));
	
			m_indices.push_back(baseIndex + ((index + 1) * ringVertexCount) + step);
			m_indices.push_back(baseIndex + ((index + 1) * ringVertexCount) + (step + 1));
			m_indices.push_back(baseIndex + (index * (ringVertexCount - 1)) + (step + baseIndex));
		}
		++baseIndex;
	}

	unsigned int southPoleIndex = m_vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (unsigned int index = 0; index < sliceCount; ++index)
	{
		m_indices.push_back(southPoleIndex - index);
		m_indices.push_back(southPoleIndex - index - (ringVertexCount + 1));
		m_indices.push_back(southPoleIndex - index - (ringVertexCount + 2));
	}
}

void Mesh::CreateInverseCube(const Vector3& position, const Vector3& extension, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	Vector3 LowerLeftFront = position + Vector3(-extension.x, -extension.y, -extension.z);
	Vector3 UpperLeftFront = position + Vector3(-extension.x, extension.y, -extension.z);
	Vector3 LowerRightFront = position + Vector3(extension.x, -extension.y, -extension.z);
	Vector3 UpperRightFront = position + Vector3(extension.x, extension.y, -extension.z);

	Vector3 LowerLeftBack = position + Vector3(-extension.x, -extension.y, extension.z);
	Vector3 UpperLeftBack = position + Vector3(-extension.x, extension.y, extension.z);
	Vector3 LowerRightBack = position + Vector3(extension.x, -extension.y, extension.z);
	Vector3 UpperRightBack = position + Vector3(extension.x, extension.y, extension.z);

	Vector3 BottomFrontNorm = LowerRightFront - LowerLeftFront;
	BottomFrontNorm.Normalize();
	Vector3 LeftFrontNorm = UpperLeftFront - LowerLeftFront;
	LeftFrontNorm.Normalize();
	Vector3 RightFrontNorm = UpperRightFront - LowerRightFront;
	RightFrontNorm.Normalize();
	Vector3 TopFrontNorm = UpperLeftFront - UpperRightFront;
	TopFrontNorm.Normalize();

	Vector3 BottomBackNorm = LowerRightBack - LowerLeftBack;
	BottomBackNorm.Normalize();
	Vector3 LeftBackNorm = UpperLeftBack - LowerLeftBack;
	LeftBackNorm.Normalize();
	Vector3 RightBackNorm = UpperRightBack - LowerRightBack;
	RightBackNorm.Normalize();
	Vector3 TopBackNorm = UpperLeftBack - UpperRightBack;
	TopBackNorm.Normalize();

	Vector3 RightBottomNorm = LowerRightBack - LowerRightFront;
	RightBottomNorm.Normalize();
	Vector3 RightTopNorm = UpperRightBack - UpperRightFront;
	RightTopNorm.Normalize();

	Vector3 LeftBottomNorm = LowerLeftBack - LowerLeftFront;
	LeftBottomNorm.Normalize();
	Vector3 LeftTopNorm = UpperLeftBack - UpperLeftFront;
	LeftTopNorm.Normalize();

	Vector3 FrontFaceNormal = CrossProduct3D(BottomFrontNorm, LeftFrontNorm);
	Vector3 BackFaceNormal = -1.0f * FrontFaceNormal;
	Vector3 RightFaceNormal = CrossProduct3D(RightBottomNorm, RightFrontNorm);
	Vector3 LeftFaceNorm = -1.0f * RightFaceNormal;
	Vector3 BottomFaceNorm = CrossProduct3D(BottomFrontNorm, LeftBottomNorm);
	Vector3 TopFaceNorm = -1.0f * BottomFaceNorm;

	m_vertices = {
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, -extension.z), color, Vector2(0.0f,1.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*0*/ //Back Face
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, -extension.z), color, Vector2(0.0f,0.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*1*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, -extension.z), color, Vector2(1.0f,0.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*2*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, -extension.z), color, Vector2(1.0f,1.0f), BackFaceNormal, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*3*/

		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*4*/ //Front Face
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*5*/
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*6*/
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f), FrontFaceNormal, Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),/*7*/

		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*8*/ //Left Face
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*9*/
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, -extension.z), color, Vector2(1.0f,0.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*10*/
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, -extension.z), color, Vector2(1.0f,1.0f), RightFaceNormal, Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*11*/

		Vertex3_PCT(position + Vector3(extension.x, -extension.y, -extension.z), color, Vector2(0.0f,1.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*12*/ //Right Face
		Vertex3_PCT(position + Vector3(extension.x, extension.y, -extension.z), color, Vector2(0.0f,0.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*13*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*14*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f), LeftFaceNorm, Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),/*15*/

		Vertex3_PCT(position + Vector3(-extension.x, extension.y, -extension.z), color, Vector2(0.0f,1.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*16*/ //Top Face
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*17*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*18*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, -extension.z), color, Vector2(1.0f,1.0f), TopFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),/*19*/

		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*20*/ //Bottom Face
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, -extension.z), color, Vector2(0.0f,0.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*21*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, -extension.z), color, Vector2(1.0f,0.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*22*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f), BottomFaceNorm, Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),/*23*/
	};

	m_indices = { 0,2,3,0,1,2, 4,6,7,4,5,6, 8,10,11,8,9,10, 12,14,15,12,13,14, 16,18,19,16,17,18, 20,22,23,20,21,22 };
}

void Mesh::WriteToStream(BinaryStream* stream)
{
	size_t vertexSize = m_vertices.size();
	size_t indexSize = m_indices.size();

	stream->write(vertexSize);
	for (uint vertexIndex = 0; vertexIndex < vertexSize; ++vertexIndex)
	{
		Vertex3_PCT vertex = m_vertices[vertexIndex];
		stream->write(vertex);
	}

	stream->write(indexSize);
	for (uint indexval = 0; indexval < indexSize; ++indexval)
	{
		unsigned int index = m_indices[indexval];
		stream->write(index);
	}
}

void Mesh::ReadFromStream(BinaryStream* stream)
{
	size_t vertexSize;
	size_t indexSize;

	stream->read(&vertexSize);
	m_vertices.reserve(vertexSize);
	for (uint vertexIndex = 0; vertexIndex < vertexSize; ++vertexIndex)
	{
		Vertex3_PCT vertex;
		stream->read(&vertex);
		m_vertices.push_back(vertex);
	}

	stream->read(&indexSize);
	m_indices.reserve(indexSize);
	for (uint indexval = 0; indexval < indexSize; ++indexval)
	{
		unsigned int index = 0;
		stream->read(&index);
		m_indices.push_back(index);
	}
}

