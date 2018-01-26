#include "Game/Behavior.hpp"
#include "Game/Wander.hpp"
#include "Game/Pursue.hpp"
#include "Game/Flee.hpp"
#include "Game/Character.hpp"
#include "Game/Attack.hpp"

Behavior::Behavior()
	:m_actingCharacter(nullptr)
	, m_didIAct(false)
{

}

Behavior::~Behavior()
{

}

Behavior* Behavior::Clone()
{
	return nullptr;
}

Behavior* Behavior::CreateBehavior(tinyxml2::XMLElement& element)
{
	m_name = element.Name();
	if (m_name == "Wander") { return new Wander(element); }
	if (m_name == "Pursue") { return new Pursue(element); }
	if (m_name == "Flee") { return new Flee(element); }
	if (m_name == "Attack") { return new Attack(element); }

	return nullptr;
}

void Behavior::Act()
{

}

void Behavior::Render() const
{

}

int Behavior::CalculateUtility()
{
	return 0;
}

void Behavior::SetActingCharacter(Character* actor)
{
	m_actingCharacter = actor;
}

std::map<std::string, std::map<std::string, Behavior*>> Behavior::s_behaviorList;

