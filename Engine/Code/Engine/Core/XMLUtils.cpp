#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <sstream>
#include <vector>


std::string ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const std::string& defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return element.Attribute(attributeName);
	}
}

std::vector<std::string> ParseXmlCommaDelimitedAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const std::string& defaultValue)
{
	std::vector<std::string> basic = { defaultValue };

	if (&element == nullptr)
	{
		return basic;
	}
	else
	{
		std::string listOfVals = element.Attribute(attributeName);
		if (listOfVals.empty())
			return basic;

		std::stringstream stream = std::stringstream(listOfVals);
		std::vector<std::string> list;
		std::string segment;

		while (std::getline(stream, segment, ','))
		{
			list.push_back(segment);
		}

		return list;
	}
}

IntVector2 ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const IntVector2& defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		std::string string = element.Attribute(attributeName);
		std::stringstream stream = std::stringstream(string);
		std::vector<int> list;
		std::string segment;

		while (std::getline(stream, segment, ','))
		{
			list.push_back(std::stoi(segment));
		}

		return IntVector2(list[0], list[1]);
	}
}

Vector2 ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const Vector2& defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		std::string string = element.Attribute(attributeName);
		std::stringstream stream = std::stringstream(string);
		std::vector<float> list;
		std::string segment;

		while (std::getline(stream, segment, ','))
		{
			list.push_back(std::stof(segment));
		}

		return Vector2(list[0], list[1]);
	}
}

Vector3 ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const Vector3& defaultValue)
{
	if (&element == nullptr || element.FindAttribute(attributeName) == nullptr)
	{
		return defaultValue;
	}
	else
	{
		std::string string = element.Attribute(attributeName);
		std::stringstream stream = std::stringstream(string);
		std::vector<float> list;
		std::string segment;

		while (std::getline(stream, segment, ','))
		{
			list.push_back(std::stof(segment));
		}

		if (list.size() < 3)
			list.push_back(0.0f);

		return Vector3(list[0], list[1], list[2]);
	}
}

Rgba ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const Rgba& defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		std::string string = element.Attribute(attributeName);
		std::stringstream stream = std::stringstream(string);
		std::vector<int> list;
		std::string segment;

		while (std::getline(stream, segment, ','))
		{
			list.push_back(std::stoul(segment));
		}

		if(list.size() < 4)
			return Rgba((unsigned char)list[0], (unsigned char)list[1], (unsigned char)list[2], 255);
		else
			return Rgba((unsigned char)list[0], (unsigned char)list[1], (unsigned char)list[2], (unsigned char)list[3]);
	}
}

float ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, float defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return element.FloatAttribute(attributeName);
	}
}

bool ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, bool defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return element.BoolAttribute(attributeName);
	}
}

char ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, char defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return *element.Attribute(attributeName);
	}
}

const char* ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, const char* defaultValue)
{
	if (&element == nullptr || element.FindAttribute(attributeName) == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return element.Attribute(attributeName);
	}
}

int ParseXmlAttribute(const tinyxml2::XMLElement& element, const char* attributeName, int defaultValue)
{
	if (&element == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return element.IntAttribute(attributeName);
	}
}

void ValidateXmlElement(const tinyxml2::XMLElement& element, const char* commaSeparatedListOfValidChildElementNames, const char* commaSeparatedListOfValidAttributeNames)
{
	std::string stringOfChildren = std::string(commaSeparatedListOfValidChildElementNames);
	std::string stringOfAttributes = std::string(commaSeparatedListOfValidAttributeNames);

	std::stringstream stringOfChildrenStream = std::stringstream(stringOfChildren);
	std::vector<std::string> stringOfChildrenList;
	std::string segment;

	while (std::getline(stringOfChildrenStream, segment, ','))
	{
		stringOfChildrenList.push_back(segment);
	}

	std::stringstream stringOfAttributesStream = std::stringstream(stringOfAttributes);
	std::vector<std::string> stringOfAttributesList;

	while (std::getline(stringOfAttributesStream, segment, ','))
	{
		stringOfAttributesList.push_back(segment);
	}

	bool wasChildNotFound = true; 
	for (unsigned int index = 0; index < stringOfChildrenList.size(); index++)
	{	
		for (auto child = element.FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			std::string name = child->Value();
			if (name != stringOfChildrenList[index])
			{
				wasChildNotFound = true;
				continue;
			}

			wasChildNotFound = false;

			for (unsigned int attr_index = 0; attr_index < stringOfAttributesList.size(); attr_index++)
			{
				auto attrPTR = child->FindAttribute(stringOfAttributesList[attr_index].c_str());
				ASSERT_OR_DIE(attrPTR == nullptr,"Attribute " + stringOfAttributesList[attr_index] + " does not exist in " + name + "!");
			}
		}

		ASSERT_OR_DIE(wasChildNotFound, "Element " + std::string(element.Value()) + " does not have child " + stringOfChildrenList[index] + "!");
	}
}

