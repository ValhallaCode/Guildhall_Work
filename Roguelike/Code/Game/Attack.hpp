#pragma once
#include "Game/Behavior.hpp"
#include "Engine/Core/XMLUtils.hpp"



class Attack : public Behavior
{
public:
	Attack();
	Attack(tinyxml2::XMLElement& element);
	virtual ~Attack();
	virtual Behavior* Clone() override;
	virtual void Act() override;
	virtual int CalculateUtility() override;
	virtual void Render() const override;
public:

};




