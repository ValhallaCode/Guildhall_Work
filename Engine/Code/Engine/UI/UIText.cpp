#include "Engine/UI/UIText.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Render/KerningFont.hpp"
#include "Engine/Render/BitmapFont.hpp"


UIText::UIText()
	:m_bitmap(nullptr)
	, m_kerning(nullptr)
	, m_atlas(nullptr)
	, m_fontScale(1.0f)
	, m_fontColor(255, 255, 255, 255)
	, m_lineSpacing(25.0f)
	, m_aspect(1.0f)
	, m_fitBounds(false)
	, m_cutoff(false)
	, m_textWrap(false)
{
	m_fontShader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);
}

UIText::~UIText()
{
	ClearOldMeshes();
}

void UIText::SetString(const std::string& text)
{
	m_string = text;
	GenerateMeshes();
}

void UIText::SetScale(float size)
{
	m_fontScale = size;
	GenerateMeshes();
}

void UIText::SetAspectRatio(float aspect)
{
	m_aspect = aspect;
	GenerateMeshes();
}

void UIText::SetLineSpacing(float spacing)
{
	m_lineSpacing = spacing;
	GenerateMeshes();
}

void UIText::SetKerningFont(KerningFont* font)
{
	if (m_bitmap)
		m_bitmap = nullptr;

	m_kerning = font;
	m_atlas = CreateOrGetTexture2D(font->m_name, g_simpleRenderer, "Data/Fonts/" + font->m_pageMap[0]);
	GenerateMeshes();
}

void UIText::SetBitmapFont(BitmapFont* font)
{
	if (m_kerning)
		m_kerning = nullptr;

	m_bitmap = font;
	m_atlas = font->GetDirectXFontTexture();
	GenerateMeshes();
}

void UIText::SetFontColor(const Rgba& color)
{
	m_fontColor = color;
	GenerateMeshes();
}

void UIText::SetTextWrapping(bool can_wrap)
{
	m_textWrap = can_wrap;
	GenerateMeshes();
}

void UIText::EnableBoundsFitToText(bool fit_to_text)
{
	m_fitBounds = fit_to_text;
	GenerateMeshes();
}

void UIText::SetBoundsCutOff(bool can_cutoff_text)
{
	m_cutoff = can_cutoff_text;
	GenerateMeshes();
}

void UIText::Render()
{
	g_simpleRenderer->MakeModelMatrixIdentity();

	Matrix4 transform = GetWorldTransform();
	g_simpleRenderer->SetModelMatrix(transform);

	#if defined(_DEBUG)
		DebugRender();
	#endif

	// Custom Logic Here
	g_simpleRenderer->SetShaderProgram(m_fontShader);
	g_simpleRenderer->SetTexture(m_atlas);

	for (Mesh* mesh : m_characters)
	{
		g_simpleRenderer->DrawMesh(*mesh);
	}

	for (UIElement* child : m_children)
	{
		child->Render();
	}
}

void UIText::GenerateMeshes()
{
	if (m_kerning)
	{
		ClearOldMeshes();
		GenerateKerningMeshes();
	}
	else if (m_bitmap)
	{
		ClearOldMeshes();
		GenerateBitmapMeshes();
	}
}

void UIText::ClearOldMeshes()
{
	if (m_characters.empty())
		return;

	for (Mesh* mesh : m_characters)
	{
		delete mesh;
		mesh = nullptr;
	}

	m_characters.clear();
}

void UIText::GenerateKerningMeshes()
{
	// Establish General Information to draw text 
	// starting from top left of bounds
	char const *c = m_string.c_str();
	int prevCharCode = -1;

	float text_height = m_kerning->GetTextHeight("T", m_fontScale) * m_aspect;
	float text_width = m_kerning->GetTextWidth("T", m_fontScale, m_aspect);
	Vector2 bounds_dim = m_bounds.CalcSize();

	Vector2 cursor = Vector2(m_bounds.mins.x, m_bounds.maxs.y);
	float string_height = m_kerning->GetTextHeight(m_string.c_str(), m_fontScale) * m_aspect;
	float total_height = string_height;
	if(m_fitBounds)
	{

		for (uint index = 0; index < m_string.size(); index++)
		{
			const char c[2] = { m_string[index], NULL };

			Glyph valid = m_kerning->GetGlyph(*c);
			const Glyph* glyph = &(valid);

			if (glyph == nullptr)
			{
				Glyph invalid = m_kerning->GetInvalidGlyph();
				glyph = &(invalid);
			}

			// Move the current cursor position to the right of this glyph
			cursor += Vector2(((float)glyph->xadvance  * m_fontScale) * m_aspect, 0);
			prevCharCode = *c;

			// Move Cursor based on Kerning relationship between me and prev Glyph
			int kerning = m_kerning->GetKerning((int)prevCharCode, (int)*c);
			cursor += Vector2(((float)kerning  * m_fontScale) * m_aspect, 0);

			// Determines whether text can wrap
			if (!m_cutoff && m_textWrap)
			{
				// Gets the Next Character and checks for wrapping
				Glyph next_valid = m_kerning->GetGlyph(*c);
				const Glyph* next_glyph = &(next_valid);

				if (next_glyph == nullptr)
				{
					Glyph invalid = m_kerning->GetInvalidGlyph();
					next_glyph = &(invalid);
				}

				float predicted_text_width = (cursor.x + (((float)next_glyph->xadvance * m_fontScale))) - m_bounds.mins.x;

				if (predicted_text_width >= bounds_dim.x)
				{
					const char* c_string = m_string.c_str();
					total_height += m_kerning->GetTextHeight(c_string + index, m_fontScale) * m_aspect;
					cursor = Vector2(m_bounds.mins.x, cursor.y) - Vector2(0.0f, text_height + m_lineSpacing);
				}
			}
		}

		// Re-fits bounds based on text calculation
		if (m_textWrap)
			SetSize(Vector2(bounds_dim.x, total_height));
		else
		{
			float new_width = m_kerning->GetTextWidth(m_string.c_str(), m_fontScale, m_aspect);
			SetSize(Vector2(new_width, string_height));
		}
	}

	cursor = Vector2(m_bounds.mins.x, m_bounds.maxs.y);

	while (*c != NULL)
	{
		// Early out for cut off
		const char list[2] = { *c , NULL};
		if ((cursor.x + m_kerning->GetTextWidth(list, m_fontScale, m_aspect) > m_bounds.maxs.x || cursor.y - text_height < m_bounds.mins.y) && m_cutoff)
			break;

		// Get current glyph and check its validity
		Glyph valid = m_kerning->GetGlyph(*c);
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
		Mesh* text_quad = new Mesh();
		text_quad->CreateOneSidedQuad(bottomLeft, topRight, uvBottomLeft, uvTopRight, m_fontColor);
		m_characters.push_back(text_quad);

		// Move the current cursor position to the right of this glyph
		cursor += Vector2(((float)glyph->xadvance  * m_fontScale) * m_aspect, 0);
		prevCharCode = *c;
		++c;

		// Move Cursor based on Kerning relationship between me and prev Glyph
		int kerning = m_kerning->GetKerning((int)prevCharCode, (int)*c);
		cursor += Vector2(((float)kerning  * m_fontScale) * m_aspect, 0);

		// Determines whether text can wrap
		if(*c != NULL && !m_cutoff && m_textWrap)
		{
			// Gets the Next Character and checks for wrapping
			Glyph next_valid = m_kerning->GetGlyph(*c);
			const Glyph* next_glyph = &(next_valid);

			if (next_glyph == nullptr)
			{
				Glyph invalid = m_kerning->GetInvalidGlyph();
				next_glyph = &(invalid);
			}

			float predicted_text_width = (cursor.x + (((float)next_glyph->xadvance * m_fontScale))) - m_bounds.mins.x;

			if (predicted_text_width >= bounds_dim.x)
			{
				float remain_text_height = m_kerning->GetTextHeight(c, m_fontScale) * m_aspect;
				cursor = Vector2(m_bounds.mins.x, cursor.y) - Vector2(0.0f, remain_text_height + m_lineSpacing);
			}
		}
	}

}

void UIText::GenerateBitmapMeshes()
{
	float cell_height = m_fontScale;
	float cell_width = cell_height * m_aspect;

	Vector2 cursor = Vector2(m_bounds.mins.x, m_bounds.maxs.y - cell_height);

	Vector2 bounds_dim = m_bounds.CalcSize();

	if (m_fitBounds)
	{
		uint line_count = 1;
		for (uint index = 0; index < m_string.size(); index++)
		{
			// Move the current cursor position to the right of this glyph
			cursor.x += cell_width;

			// Determines whether text can wrap
			if (cursor.x + cell_width >= m_bounds.maxs.x && m_textWrap)
			{
				line_count++;
				cursor = Vector2(m_bounds.mins.x, cursor.y) - Vector2(0.0f, cell_height + m_lineSpacing);
			}
		}

		// Re-fits bounds based on text calculation
		float bounds_height = bounds_dim.y;
		float new_height = line_count * cell_height;

		if (m_textWrap)
			SetSize(Vector2(bounds_dim.x, new_height));
		else
		{
			float new_width = cell_width * m_string.size();//m_bounds.mins.x + cursor.x;
			SetSize(Vector2(new_width, new_height));
		}
	}

	cursor = Vector2(m_bounds.mins.x, m_bounds.maxs.y - cell_height);

	for (uint index = 0; index < m_string.size(); index++)
	{
		// Early out for cut off
		if ((cursor.x + cell_width > m_bounds.maxs.x || cursor.y < m_bounds.mins.y) && m_cutoff)
			break;

		AABB2D fontTexCoords = m_bitmap->GetTexCoordsForGlyph((int)m_string[index]);
		AABB2D worldBounds(cursor, Vector2(cursor.x + cell_width, cursor.y + cell_height));

		// Gen Meshes
		Mesh* text_quad = new Mesh();
		text_quad->CreateOneSidedQuad(worldBounds.mins, worldBounds.maxs, fontTexCoords.mins, fontTexCoords.maxs, m_fontColor);
		m_characters.push_back(text_quad);

		cursor.x += cell_width;

		// Determines whether text can wrap
		if (cursor.x + cell_width >= m_bounds.maxs.x && m_textWrap)
		{
			cursor = Vector2(m_bounds.mins.x, cursor.y) - Vector2(0.0f, cell_height + m_lineSpacing);
		}
	}
}