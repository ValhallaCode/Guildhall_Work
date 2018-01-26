#pragma once
#include "Engine/UI/UIElement.hpp"

class Texture2D;

class UICanvas : public UIElement
{
public:
	UICanvas();
	virtual ~UICanvas();
	void SetTexture(Texture2D* tex);
	void SetTexture(const char* file_path);
	void SetTargetResolution(float target);
	virtual void Render();
public:
	Texture2D* m_texture;
	Vector2 m_resolution;
};