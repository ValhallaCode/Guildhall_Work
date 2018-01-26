#pragma once
#include "Game/TileDescription.hpp"
#include "Game/Inventory.hpp"
#include "Game/Entity.hpp"
#include "Game/Character.hpp"
#include "Game/Interactable.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/RHI/Sampler.hpp"




enum Visibility : unsigned int
{
	HAS_NOT_SEEN,
	CAN_SEE,
	HAS_SEEN,
};


class Tile
{
public:
	Tile();
	Tile(TileDescription* description);
	~Tile();
	void SetTileDescription(TileDescription* tileDesc);
	TileDescription* GetTileDescription() const;
	void SetEntityOnTile(Entity* entity);
	Character* GetCharacterOnTile() const;
	Interactable* GetInteractableOnTile() const;
	void SetPositionInMap(IntVector2& position);
	void SetPositionInMap(int positionX, int positionY);
	IntVector2 GetPositionInMap() const;
	void Render() const;
	unsigned char GetAlphaValueForLightValue();
public:
	int m_closedPathID;
	int m_openPathID;
	int m_lightValue;
	float m_localG;
	unsigned int m_variantIndex;
	TileDescription* m_description;
	Inventory m_inventory;
	Character* m_characterOnTile;
	Interactable* m_interactableOnTile;
	IntVector2 m_positionInMap;
	//Drawing Variables
	BitmapFont* m_font;
	ShaderProgram* m_shaderProgram;
	Sampler* m_sampler;
	Mesh* m_mesh;
	AABB2D m_bounds;
	Visibility m_visibleState;
};