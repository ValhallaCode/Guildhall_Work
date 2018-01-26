#include "Game/Tile.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Math/MathUtils.hpp"

Tile::Tile()
{

}
Tile::Tile(TileDescription* description)
	:m_characterOnTile(nullptr)
	, m_interactableOnTile(nullptr)
	, m_closedPathID(-1)
	, m_openPathID(-1)
	, m_localG(1.0f)
	, m_visibleState(HAS_NOT_SEEN)
	, m_lightValue(10)
{
	SetTileDescription(description);
	m_font = g_simpleRenderer->CreateOrGetBitmapFont("PixelFont");
	m_sampler = g_simpleRenderer->CreateOrGetSampler("PointToPoint", eFilterMode::FILTER_POINT, eFilterMode::FILTER_POINT);
	m_shaderProgram = g_simpleRenderer->CreateOrGetShaderProgram("Data/HLSL/nop_textured.hlsl");
	m_mesh = new Mesh();

	m_variantIndex = GetRandomIntInRange(0, (int)m_description->m_color.size() - 1);
}

Tile::~Tile()
{

}

void Tile::SetTileDescription(TileDescription* tileDesc)
{
	m_description = tileDesc;
}

TileDescription* Tile::GetTileDescription()  const
{
	return m_description;
}

void Tile::SetEntityOnTile(Entity* entity)
{
	Character* character = dynamic_cast<Character*>(entity);
	Interactable* interactable = dynamic_cast<Interactable*>(entity);

	if (interactable != nullptr)
	{
		m_interactableOnTile = interactable;
	}

	m_characterOnTile = character;
}

Character* Tile::GetCharacterOnTile()  const
{
	return m_characterOnTile;
}

Interactable* Tile::GetInteractableOnTile()  const
{
	return m_interactableOnTile;
}

void Tile::SetPositionInMap(IntVector2& position)
{
	m_positionInMap = position;
	m_mesh->CreateOneSidedQuad(Vector2((float)position.x, (float)position.y), Vector2((float)position.x + 1.0f, (float)position.y + 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), m_description->m_color[m_variantIndex]);
	m_bounds = AABB2D(Vector2((float)position.x, (float)position.y), Vector2((float)position.x + 1.0f, (float)position.y + 1.0f));
}

void Tile::SetPositionInMap(int positionX, int positionY)
{
	m_positionInMap = IntVector2(positionX, positionY);
	m_mesh->CreateOneSidedQuad(Vector2((float)positionX, (float)positionY), Vector2((float)positionX + 1.0f, (float)positionY + 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), m_description->m_color[m_variantIndex]);
	m_bounds = AABB2D(Vector2((float)positionX, (float)positionY), Vector2((float)positionX + 1.0f, (float)positionY + 1.0f));
}

IntVector2 Tile::GetPositionInMap() const
{
	return m_positionInMap;
}

void Tile::Render() const
{
	if (m_visibleState == HAS_NOT_SEEN)
		return;

	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->SetShaderProgram(m_shaderProgram);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);
	
	int memoryColor;
	if (m_visibleState == HAS_SEEN)
	{
		memoryColor = -3;
		if (m_lightValue + memoryColor < 0)
			memoryColor = 0;
	}
	else
		memoryColor = 0;


	unsigned char alpha;

	switch (m_lightValue + memoryColor)
	{
	case 0: alpha = 90; break;
	case 1: alpha = 95; break;
	case 2: alpha = 100; break;
	case 3: alpha = 108; break;
	case 4: alpha = 115; break;
	case 5: alpha = 128; break;
	case 6: alpha = 148; break;
	case 7: alpha = 188; break;
	case 8: alpha = 214; break;
	case 9: alpha = 235; break;
	default: alpha = 255; break;
	}

	if (m_visibleState == HAS_SEEN)
	{
		Rgba color(m_description->m_color[m_variantIndex].r, m_description->m_color[m_variantIndex].g, m_description->m_color[m_variantIndex].b, alpha); //128
		m_mesh->m_indices.clear();
		m_mesh->m_vertices.clear();
		m_mesh->CreateOneSidedQuad(Vector2((float)m_positionInMap.x, (float)m_positionInMap.y), Vector2((float)m_positionInMap.x + 1.0f, (float)m_positionInMap.y + 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), color);
	}
	else
	{
		m_mesh->m_indices.clear();
		m_mesh->m_vertices.clear();
		Rgba color(m_description->m_color[m_variantIndex].r, m_description->m_color[m_variantIndex].g, m_description->m_color[m_variantIndex].b, alpha); //255
		m_mesh->CreateOneSidedQuad(Vector2((float)m_positionInMap.x, (float)m_positionInMap.y), Vector2((float)m_positionInMap.x + 1.0f, (float)m_positionInMap.y + 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), color);
	}

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawMesh(*m_mesh);
	g_simpleRenderer->DisableBlend();

	if ((m_characterOnTile != nullptr || m_interactableOnTile != nullptr))
		return;

	if(((m_characterOnTile == nullptr && m_interactableOnTile == nullptr) || m_visibleState == HAS_SEEN) && m_inventory.m_itemList.empty())
	{
		g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

		if (m_visibleState == HAS_SEEN)
		{
			Rgba color(m_description->m_charColor[m_variantIndex].r, m_description->m_charColor[m_variantIndex].g, m_description->m_charColor[m_variantIndex].b, 128); //------------------------------------------HAS_SEEN COLOR // 128
			g_simpleRenderer->DrawBitmapFont(m_font, m_bounds.mins + Vector2(-0.09f, -0.1f), m_description->m_char[m_variantIndex], m_bounds.CalcSize().y, color, 0.8f);
		}
		else
		{
			g_simpleRenderer->DrawBitmapFont(m_font, m_bounds.mins + Vector2(-0.09f, -0.1f), m_description->m_char[m_variantIndex], m_bounds.CalcSize().y, m_description->m_charColor[m_variantIndex], 0.8f);
		}
		g_simpleRenderer->DisableBlend();
	}
	else if (m_characterOnTile == nullptr && m_interactableOnTile == nullptr && m_visibleState != HAS_SEEN)
	{
		const Item* item = m_inventory.m_itemList[0];
		g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
		g_simpleRenderer->DrawBitmapFont(m_font, m_bounds.mins + Vector2(-0.09f, -0.1f), item->m_charToDraw, m_bounds.CalcSize().y, item->m_charColor, 0.8f);
		g_simpleRenderer->DisableBlend();
	}
}

unsigned char Tile::GetAlphaValueForLightValue()
{
	switch (m_lightValue)
	{
	case 0: return 90;
	case 1: return 95;
	case 2: return 100;
	case 3: return 108;
	case 4: return 115;
	case 5: return 128;
	case 6: return 148;
	case 7: return 188;
	case 8: return 214;
	case 9: return 235;
	default: return 255;
	}
}