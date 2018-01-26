#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Game/Stat.hpp"
#include "Game/Inventory.hpp"
#include <string>

class Tile;
class Map;
class Character;

struct DamageIndicator
{
	std::string m_damage;
	Rgba m_color;
	Vector2 m_position;
};


class Entity
{
public:
	Entity();
	~Entity();
	virtual void Update(float deltaSeconds);
	void SetCurrentTileOn(Tile* tile);
	Tile* GetCurrentTileOn();
	virtual void UpdateBoxPosition(IntVector2& position);
	IntVector2 GetPositionInMap();
	void SetCurrentMap(Map* map);
	Map* GetCurrentMap();
	virtual void Render() const;
	virtual void DropItem(Item& item, Inventory& inventoryToAdd);
	virtual void PickupItem(Item& item, Inventory& inventoryToRemove);
	virtual void UpdateDamageNumbers(float deltaSeconds);
	virtual void RemoveOldDamageNumbers();
	virtual void RenderDamageNumbers() const;
	virtual Stat GetBaseWithoutEquipmentStats();
	void PlayerCheckForVisibleTiles();
	void LookForActorsInView();
public:
	Stat* m_stats;
	Inventory* m_inventory;
	Tile* m_currentTile;
	Map* m_currentMap;
	std::string m_name;
	//Drawing values
	std::string m_charToDraw;
	Rgba m_charColor;
	BitmapFont* m_font;
	ShaderProgram* m_shaderProgram;
	Sampler* m_sampler;
	AABB2D m_bounds;
	std::vector<DamageIndicator> m_damageNumbers;
	std::vector<IntVector2> m_seenTiles;
	std::vector<Character*> m_actorsSeen;
	IntVector2 m_location;
};