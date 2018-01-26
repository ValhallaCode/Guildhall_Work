#include "Game/Inventory.hpp"
#include <algorithm>



bool Inventory::RemoveItem(const Item* item) {
	auto found_iter = std::find(m_itemList.begin(), m_itemList.end(), item);
	if (found_iter != m_itemList.end()) {
		m_itemList.erase(found_iter);
		return true;
	}
	return false;
}

bool Inventory::RemoveItemByName(const Item* item) {
	auto found_iter = std::find_if(m_itemList.begin(), m_itemList.end(), [&](const Item* i)->bool { return i->m_name == item->m_name; });
	if (found_iter != m_itemList.end()) {
		m_itemList.erase(found_iter);
		return true;
	}
	return false;
}

Inventory::Inventory()
{

}

Inventory::Inventory(unsigned int maxSize)
	:m_maxItemCount(maxSize)
{

}

Inventory::~Inventory()
{

}
