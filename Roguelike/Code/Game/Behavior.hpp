#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include <string>
#include <map>

class Character;

class Behavior
{
public:
	Behavior();
	virtual ~Behavior();
	virtual Behavior* Clone();
	Behavior* CreateBehavior(tinyxml2::XMLElement& element);
	virtual void Act();
	virtual void Render() const;
	virtual int CalculateUtility();
	void SetActingCharacter(Character* actor);
public:
	bool m_didIAct;
	std::string m_name;
	Character* m_actingCharacter;
	static std::map<std::string, std::map<std::string, Behavior*>> s_behaviorList;
};