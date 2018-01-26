#pragma once
#include "Game/Entity.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Game/InteractableDescription.hpp"

class Interactable : public virtual Entity
{
public:
	Interactable();
	virtual ~Interactable();
	void UpdateBoxPosition(IntVector2& position) override;
	virtual void Update(float deltaSeconds) override;
	Interactable(InteractableDescription* interacDesc);
	virtual void Render() const override;
public:
	Mesh* m_mesh;
	Rgba m_color;
	std::string m_tileNameToPlace;
	std::string m_destinationMap;
	std::string m_destinationFeature;
	IntVector2 m_exitLocation;
	int m_lightValue;
	bool m_isLight;
	bool m_interacted;
	bool m_canBeDrawn;
	bool m_isSolid;
	bool m_isOpaque;
	bool m_isExit;
};