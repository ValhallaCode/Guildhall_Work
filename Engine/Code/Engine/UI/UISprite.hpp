#pragma once
#include "Engine/UI/UIElement.hpp"

class Sprite;
class Texture2D;

class UISprite : public UIElement
{
public:
	UISprite();
	virtual ~UISprite();
	void SetTexture(Texture2D* tex);
	void SetTexture(const char* file_path);
	void SetBounds(const AABB2D& bounds);
	virtual void Render();
public:
	Sprite* m_sprite;
};