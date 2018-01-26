#include "Engine/UI/UIEditableText.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Render/KerningFont.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/Render/Vertex.hpp"
#include <vector>

UIEditableText::UIEditableText()
	:m_cursorIndex(0)
	, m_addedCharacterToEnd(false)
{
	m_cursorMesh = new Mesh();
}

UIEditableText::~UIEditableText()
{

}

void UIEditableText::MoveCursorRight()
{
	SetCursorLocation(m_cursorIndex + 1);
}

void UIEditableText::MoveCursorLeft()
{
	SetCursorLocation(m_cursorIndex - 1);
}

void UIEditableText::SetBlinksPerSecond(float blinks)
{
	m_cursorInterval.SetFrequency(blinks);
	GenerateCursorMesh();
}

void UIEditableText::SetCursorLocation(int index)
{
	uint string_size = 0;
	if (!m_string.empty())
		string_size = m_string.size() - 1;

	SetTranslationBasedOnIndex(index);

	if (m_cursorDrawIndex != m_string.size())
		m_cursorIndex = ClampWithin(index, string_size, 0);
	
	m_cursorDrawIndex = ClampWithin(index, m_string.size(), 0);
	
	GenerateCursorMesh();
}

void UIEditableText::InsertCharacter(char new_char)
{
	if (m_cursorDrawIndex == m_string.size())
	{
		m_string.push_back(new_char);
		m_addedCharacterToEnd = true;
		SetCursorLocation(m_string.size());
	}
	else
	{
		m_string.insert(m_cursorIndex, std::string(&new_char));
		GenerateCursorMesh();
		SetCursorLocation(m_cursorIndex + 1);
	}

	GenerateMeshes();
}

void UIEditableText::RemoveCharacter()
{
	if (m_string.empty())
		return;

	m_string.erase(m_cursorIndex, 1);
	GenerateMeshes();
	SetCursorLocation(m_cursorIndex);
}

void UIEditableText::Render()
{
	if (m_cursorInterval.CheckAndReset())
	{
		m_canDrawCursor = !m_canDrawCursor;
	}

	g_simpleRenderer->MakeModelMatrixIdentity();

	Matrix4 transform = GetWorldTransform();
	g_simpleRenderer->SetModelMatrix(transform);

	#if defined(_DEBUG)
		DebugRender();
	#endif

	// Custom Logic Here
	g_simpleRenderer->SetShaderProgram(m_fontShader);
	g_simpleRenderer->SetTexture(m_atlas);

	Matrix4 string_trans = transform;
	string_trans.Translate(m_cursorTranslation);
	g_simpleRenderer->MakeModelMatrixIdentity();
	g_simpleRenderer->SetModelMatrix(string_trans);

	for (Mesh* mesh : m_characters)
	{
		g_simpleRenderer->DrawMesh(*mesh);
	}

	if (m_canDrawCursor)
	{
		g_simpleRenderer->DrawMesh(*m_cursorMesh);
	}

	for (UIElement* child : m_children)
	{
		child->Render();
	}
}

void UIEditableText::Reset()
{
	m_string.clear();
	GenerateMeshes();
	SetCursorLocation(0);
}

int UIEditableText::GetLength()
{
	return m_string.size();
}

bool UIEditableText::IsEmpty()
{
	return m_string.empty();
}

std::string UIEditableText::GetString()
{
	return m_string;
}

void UIEditableText::GenerateCursorMesh()
{
	m_cursorMesh->m_vertices.clear();
	m_cursorMesh->m_indices.clear();

	float cursor_side = 1.0f;
	if (m_cursorDrawIndex == m_string.size())
		cursor_side = -1.0f;

	if (m_kerning)
	{
		// Determines where cursor is on the string
		float char_width;
		if (!m_string.empty()) 
		{
			char c[2] = { m_string[m_cursorIndex], NULL };
			char_width = m_kerning->GetTextWidth(c, m_fontScale, m_aspect);
		}
		else
			char_width = 0.0f;

		std::string str_til_cursor = std::string(m_string.begin(), m_string.begin() + m_cursorIndex);
		float len_to_cursor = m_kerning->GetTextWidth(str_til_cursor.c_str(), m_fontScale, m_aspect);
		Vector2 cursor = Vector2(0.0f, m_bounds.maxs.y);
		cursor.x = m_bounds.mins.x + len_to_cursor - ((char_width * 0.5f) * cursor_side);

		// Get current glyph and check its validity
		Glyph valid = m_kerning->GetGlyph('|');
		const Glyph* glyph = &(valid);

		if (glyph == nullptr)
		{
			Glyph invalid = m_kerning->GetInvalidGlyph();
			glyph = &(invalid);
		}

		Vector2 bottomLeft;
		Vector2 topRight;

		// Determine Positioning Information for Glyph's Quad
		Vector2 topLeft = cursor + Vector2(((float)glyph->xoffset * m_fontScale) * m_aspect, ((float)-glyph->yoffset * m_fontScale) * m_aspect);
		bottomLeft = topLeft + Vector2(0, ((float)-glyph->height  * m_fontScale) * m_aspect);
		topRight = topLeft + Vector2(((float)glyph->width * m_fontScale) * m_aspect, 0);

		// Determine UVs for selected Glyph
		Vector2 uvTopLeft = Vector2((float)glyph->x / (float)m_kerning->m_scaleW, (float)glyph->y / (float)m_kerning->m_scaleH);
		Vector2 uvBottomLeft = uvTopLeft + Vector2(0, (float)glyph->height / (float)m_kerning->m_scaleH);
		Vector2 uvTopRight = uvTopLeft + Vector2((float)glyph->width / (float)m_kerning->m_scaleW, 0);

		// Generate mesh for this Glyph and store it
		m_cursorMesh->CreateOneSidedQuad(bottomLeft, topRight, uvBottomLeft, uvTopRight, m_fontColor);
	}
	else if(m_bitmap)
	{
		float cell_height = m_fontScale;
		float cell_width = cell_height * m_aspect;
		//float char_width;
		//if (!m_string.empty())
		//	char_width = cell_width * 0.0f;
		//else
		//	char_width = 0.0f;

		float len_to_cursor = m_cursorIndex * cell_width;
		Vector2 cursor = Vector2(m_bounds.mins.x + len_to_cursor, m_bounds.maxs.y - cell_height);
		AABB2D fontTexCoords = m_bitmap->GetTexCoordsForGlyph((int)'|');
		AABB2D worldBounds(cursor, Vector2(cursor.x + cell_width, cursor.y + cell_height));

		m_cursorMesh->CreateOneSidedQuad(worldBounds.mins, worldBounds.maxs, fontTexCoords.mins, fontTexCoords.maxs, m_fontColor);
	}
}

void UIEditableText::SetTranslationBasedOnIndex(int new_index)
{
	// Don't translate if empty or greater than string size
	if (m_string.empty() || new_index <= 0)
	{
		m_cursorTranslation = Vector3(0.0f, 0.0f, 0.0f);
		return;
	}

	// Don't translate if string is shorter than bounds
	float bounds_width = m_bounds.CalcSize().x;
	float string_width;
	if (m_kerning)
	{
		string_width = m_kerning->GetTextWidth(m_string.c_str(), m_fontScale, m_aspect);
	}
	else
	{
		string_width = m_string.size() * (m_fontScale * m_aspect);
	}

	if (string_width < bounds_width)
	{
		m_cursorTranslation = Vector3(0.0f, 0.0f, 0.0f);
		return;
	}

	uint index_to_use;
	if (new_index == m_string.size())
		index_to_use = new_index - 1;
	else
		index_to_use = new_index;

	if (m_kerning)
	{
		std::string str_til_cursor = std::string(m_string.begin(), m_string.begin() + index_to_use);
		float begin_str_to_cursor_width = m_kerning->GetTextWidth(str_til_cursor.c_str(), m_fontScale, m_aspect);
		std::string str_past_cursor = std::string(m_string.begin() + index_to_use, m_string.end());
		float width_to_string_end = m_kerning->GetTextWidth(str_past_cursor.c_str(), m_fontScale, m_aspect);

		if (new_index > m_cursorIndex) // Cursor moves Right
		{
			// Moves Left
			if (begin_str_to_cursor_width >= bounds_width * 0.67f && width_to_string_end > bounds_width * 0.33f)
			{
				char c[2] = { m_string[index_to_use], NULL };
				float current_char_width = m_kerning->GetTextWidth(c, m_fontScale, m_aspect);
				m_cursorTranslation.x -= current_char_width;
			}
			else if (m_addedCharacterToEnd) // Move left becasue new character
			{
				char c[2] = { m_string[index_to_use], NULL };
				float current_char_width = m_kerning->GetTextWidth(c, m_fontScale, m_aspect);
				m_cursorTranslation.x -= current_char_width;
				m_addedCharacterToEnd = false;
			}
		}
		else if (new_index < m_cursorIndex) // Cursor moves Left
		{
			// Moves Right
			if (begin_str_to_cursor_width >= bounds_width * 0.33f && width_to_string_end >= bounds_width * 0.67f)
			{
				char c[2] = { m_string[m_cursorIndex], NULL };
				float current_char_width = m_kerning->GetTextWidth(c, m_fontScale, m_aspect);
				m_cursorTranslation.x += current_char_width;
			}
		}
	}
	else
	{
		float cell_width = m_fontScale * m_aspect;
		float begin_str_to_cursor_width = new_index * cell_width;
		float width_to_string_end = (m_string.size() - new_index) * cell_width;
		//float cursor_dir = (float)(new_index - m_cursorIndex) / std::abs((float)(new_index - m_cursorIndex));

		if (new_index > m_cursorIndex)
		{
			// Moves Right
			if (begin_str_to_cursor_width >= bounds_width * 0.67f && width_to_string_end > bounds_width * 0.33f)
			{
				m_cursorTranslation.x += cell_width;
			}
		}
		else if (new_index < m_cursorIndex)
		{
			// Moves Left
			if (begin_str_to_cursor_width > bounds_width * 0.33f)
			{
				m_cursorTranslation.x -= cell_width;
			}
		}
	}
}
