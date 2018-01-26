#include "Engine/Core/Profiler.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Render/SimpleRenderer.hpp"

Profiler::Profiler()
	:m_fillColor(Rgba(0,255,0,255))
	, m_fontColor(Rgba(255,255,255,255))
	, m_stateColor(Rgba(255, 255, 255, 255))
	, m_font(nullptr)
	, m_graphBox(AABB2D())
	, m_historySize(0)
	, m_isActive(false)
	, m_sampler(nullptr)
	, m_fontShader(nullptr)
	, m_defaultShader(nullptr)
	, m_textLocation(Vector2(0.0f,0.0f))
{

}

Profiler::~Profiler()
{

}

void Profiler::InsertIntoAllocationHistory(data_point& data)
{
	if (m_allocationHistory.size() + 1 > m_historySize) 
	{
		m_allocationHistory.erase(m_allocationHistory.begin());
		m_allocationHistory.reserve(1);
	}

	m_allocationHistory.push_back(data);
}

void Profiler::UpdateAllocationHistory(float deltaSeconds)
{
	float rateToMove = (m_graphBox.maxs.x - m_graphBox.mins.x) / ((float)m_historySize * 0.0168f);

	for (unsigned int index = 0; index < m_allocationHistory.size(); ++index)
	{
		data_point& currentData = m_allocationHistory[index];
		currentData.position.y = RangeMapFloat(0.0f, (float)GetHighwaterAllocation(), m_graphBox.mins.y, m_graphBox.maxs.y, (float)currentData.frame_alloc);
		currentData.position.x -= rateToMove * deltaSeconds;
	}
}

void Profiler::Update(float deltaSeconds)
{
	if (!m_isActive)
		return;

	// Get latest data point
	data_point newestData;
	newestData.frame_alloc = GetFrameAllocs();
	newestData.position = Vector2(m_graphBox.maxs.x - 0.1f, 0.0f);

	// Insert latest Data
	InsertIntoAllocationHistory(newestData);

	// Update List
	UpdateAllocationHistory(deltaSeconds);
}

void Profiler::Render(SimpleRenderer* renderer) const
{
	if (!m_isActive)
		return;

	renderer->SetSampler(m_sampler);
	renderer->SetDiffuse(renderer->m_whiteTexture);
	renderer->SetShaderProgram(m_defaultShader);

	// Draw Background Box for Graph
	Vector2 boxCenter = m_graphBox.CalcCenter();
	float x_radius = (m_graphBox.maxs.x - m_graphBox.mins.x) / 2.0f;
	float y_radius = (m_graphBox.maxs.y - m_graphBox.mins.y) / 2.0f;
	Mesh mesh;
	mesh.CreateOneSidedQuad(Vector3(boxCenter.x, boxCenter.y, 0.0f), Vector3(x_radius, y_radius, 0.0f), Rgba(124, 124, 124, 255));
	renderer->DrawMesh(mesh);

	#if defined(TRACK_MEMORY)
		// Draw Fill for Graph
		if (m_allocationHistory.size() > 1)
		{
			for (unsigned int index = 1; index < m_allocationHistory.size(); ++index)
			{
				data_point currentData = m_allocationHistory[index];
				data_point prevData = m_allocationHistory[index - 1];
	
				Mesh graphMesh;
				Vector2 bottomLeft(prevData.position.x, m_graphBox.mins.y + 0.1f);
				Vector2 topRight = currentData.position + Vector2(0.0f, 1.0f);
				graphMesh.CreateOneSidedQuad(bottomLeft, topRight, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), m_fillColor);
				renderer->DrawMesh(graphMesh);
			}
		}
	#endif

	// Draw Readable Info
	renderer->SetShaderProgram(m_fontShader);
	renderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

	renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x, m_textLocation.y + (3.0f * m_font->GetTextHeight("text", 1.0f)) + 10.0f), "PROFILING", m_fontColor);
	renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x, m_textLocation.y + (2.0f * m_font->GetTextHeight("text", 1.0f)) + 10.0f), "MEMORY", m_fontColor);

	#if defined (TRACK_MEMORY)
		#if (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
			renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x + m_font->GetTextWidth("MEMORY  "), m_textLocation.y + (2.0f * m_font->GetTextHeight("text", 1.0f)) + 10.0f), "[VERBOSE]", m_stateColor);
		#elif (TRACK_MEMORY == TRACK_MEMORY_BASIC)
			renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x + m_font->GetTextWidth("MEMORY  "), m_textLocation.y + (2.0f * m_font->GetTextHeight("text", 1.0f)) + 10.0f), "[BASIC]", m_stateColor);
		#endif
		
		renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x + m_font->GetTextWidth("   "), m_textLocation.y + m_font->GetTextHeight("text", 1.0f) + 10.0f), "Alloc Count: " + std::to_string(GetAllocCount()), m_fontColor);
		renderer->DrawTextCenteredOnPosition2D(m_font, m_textLocation + Vector2((float)m_font->GetTextWidth("   "), 0.0f), "Byte Count: " + std::to_string(GetAllocByteCount()), m_fontColor);
		renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x + m_font->GetTextWidth("   "), m_textLocation.y - m_font->GetTextHeight("text", 1.0f) - 10.0f), "Highwater: " + std::to_string(GetHighwaterAllocation()), m_fontColor);
		renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x + m_font->GetTextWidth("   "), m_textLocation.y - (2.0f * m_font->GetTextHeight("text", 1.0f)) - 10.0f), "Allocs Last Frame: " + std::to_string(GetFrameAllocs()), m_fontColor);
		renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x + m_font->GetTextWidth("   "), m_textLocation.y - (3.0f * m_font->GetTextHeight("text", 1.0f)) - 10.0f), "Frees Last Frame: " + std::to_string(GetFrameFrees()), m_fontColor);
	#else
		renderer->DrawTextCenteredOnPosition2D(m_font, Vector2(m_textLocation.x + m_font->GetTextWidth("MEMORY  "), m_textLocation.y + (2.0f * m_font->GetTextHeight("text", 1.0f)) + 10.0f), "[DISABLED]", m_stateColor);
	#endif

	renderer->DisableBlend();
}

void Profiler::SetGraphBox(const Vector2& center, float x_radius, float y_radius)
{
	m_graphBox = AABB2D(center, x_radius, y_radius);
}

void Profiler::SetColors(const Rgba& graphFill /*= Rgba(0, 255, 0, 255)*/, const Rgba& stateColor /*= Rgba(255, 255, 255, 255)*/, const Rgba& fontColor /*= Rgba(255, 255, 255, 255)*/)
{
	m_fillColor = graphFill;
	m_stateColor = stateColor;
	m_fontColor = fontColor;
}

void Profiler::SetFont(const std::string& filePath)
{
	m_font = CreateOrGetKerningFont(filePath);
}

void Profiler::SetHistorySize(const size_t& size)
{
	m_historySize = size;
	m_allocationHistory.reserve(size);
}

void ConsoleToggle(const std::string& args)
{
	args;
	g_profiler->m_isActive = !g_profiler->m_isActive;
}

void Profiler::SetSampler(const std::string& name, SimpleRenderer* renderer, const eFilterMode& minFilter, const eFilterMode& magFilter)
{
	m_sampler = CreateOrGetSampler(name, renderer, minFilter, magFilter);
}

void Profiler::SetFontShaderProgram(const std::string& name, const std::string& filePath, SimpleRenderer* renderer)
{
	m_fontShader = CreateOrGetShaderProgram(name, filePath, renderer);
}

void Profiler::SetDefaultShaderProgram(const std::string& name, const std::string& filePath, SimpleRenderer* renderer)
{
	m_defaultShader = CreateOrGetShaderProgram(name, filePath, renderer);
}

void Profiler::SetTextLocation(const Vector2& position)
{
	m_textLocation = position;
}
