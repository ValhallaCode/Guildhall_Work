#pragma once
#include "Game/Item.hpp"
#include <vector>

class Inventory
{
public:
	bool RemoveItem(const Item* item);
	bool RemoveItemByName(const Item* item);
	Inventory();
	Inventory(unsigned int maxSize);
	~Inventory();
public:
	std::vector<Item*> m_itemList;
	unsigned int m_maxItemCount;
};