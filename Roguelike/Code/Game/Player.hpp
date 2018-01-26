#pragma once
#include "Game/Character.hpp"
#include "Game/CharacterDescription.hpp"
#include <vector>

class Player : public Character 
{
public:
	Player();
	virtual ~Player();
	Player(CharacterDescription* charDesc);
	virtual void Update(float deltaSeconds) override;
	virtual void Act() override;
	virtual void Render() const override;
public:
	std::vector<std::string> m_killList;
};