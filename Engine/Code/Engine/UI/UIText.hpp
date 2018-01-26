#pragma once
#include "Engine/UI/UIPanel.hpp"

class KerningFont;
class BitmapFont;
class Texture2D;
class Mesh;
class ShaderProgram;

class UIText : public UIPanel
{
public:
	UIText();
	virtual ~UIText();
	void SetString(const std::string& text);
	void SetScale(float size);
	void SetAspectRatio(float aspect);
	void SetLineSpacing(float spacing);
	void SetKerningFont(KerningFont* font);
	void SetBitmapFont(BitmapFont* font);
	void SetFontColor(const Rgba& color);
	void SetTextWrapping(bool can_wrap);
	void EnableBoundsFitToText(bool fit_to_text);
	void SetBoundsCutOff(bool can_cutoff_text);
	virtual void Render() override;
protected:
	void GenerateMeshes();
	void ClearOldMeshes();
	void GenerateKerningMeshes();
	void GenerateBitmapMeshes();
protected:
	std::string m_string;
	KerningFont* m_kerning;
	BitmapFont* m_bitmap;
	Texture2D* m_atlas;
	std::vector<Mesh*> m_characters;
	float m_fontScale;
	float m_lineSpacing;
	float m_aspect;
	Rgba m_fontColor;
	bool m_textWrap;
	bool m_fitBounds;
	bool m_cutoff;
	ShaderProgram* m_fontShader;
};