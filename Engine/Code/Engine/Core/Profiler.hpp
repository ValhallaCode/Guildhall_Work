#pragma once
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/RHI/Sampler.hpp"
#include <vector>

class KerningFont;
class SimpleRenderer;
class ShaderProgram;

typedef unsigned int uint;

struct data_point
{
	uint frame_alloc;
	Vector2 position;
};

class Profiler
{
public:
	Profiler();
	~Profiler();
	void InsertIntoAllocationHistory(data_point& data);
	void UpdateAllocationHistory(float deltaSeconds);
	void Update(float deltaSeconds);
	void Render(SimpleRenderer* renderer) const;
	void SetGraphBox(const Vector2& center, float x_radius, float y_radius);
	void SetColors(const Rgba& graphFill = Rgba(0, 255, 0, 255), const Rgba& stateColor = Rgba(255, 255, 255, 255), const Rgba& fontColor = Rgba(255, 255, 255, 255));
	void SetFont(const std::string& filePath);
	void SetHistorySize(const size_t& size);
	void SetSampler(const std::string& name, SimpleRenderer* renderer, const eFilterMode& minFilter, const eFilterMode& magFilter);
	void SetFontShaderProgram(const std::string& name, const std::string& filePath, SimpleRenderer* renderer);
	void SetDefaultShaderProgram(const std::string& name, const std::string& filePath, SimpleRenderer* renderer);
	void SetTextLocation(const Vector2& position);
public:
	AABB2D m_graphBox;
	std::vector<data_point> m_allocationHistory;
	KerningFont* m_font;
	Rgba m_fillColor;
	Rgba m_stateColor;
	Rgba m_fontColor;
	size_t m_historySize;
	bool m_isActive;
	Sampler* m_sampler;
	ShaderProgram* m_fontShader;
	ShaderProgram* m_defaultShader;
	Vector2 m_textLocation;
};

void ConsoleToggle(const std::string& args);