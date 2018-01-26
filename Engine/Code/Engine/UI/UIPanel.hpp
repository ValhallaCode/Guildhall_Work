#pragma once
#include "Engine/UI/UIElement.hpp"



class UIPanel : public UIElement
{
public:
	UIPanel();
	virtual ~UIPanel();
	virtual void Render();
};