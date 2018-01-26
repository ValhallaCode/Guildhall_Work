#pragma once
#include "Engine/Config.hpp"
#include "Engine/Input/FileUtilities.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Render/Rgba.hpp"
#include <cctype>
#include <algorithm>
#include <sstream>

Config::Config(const std::string &filePath)
{
	ConfigSystemStartup(filePath);
}

Config::~Config()
{
	ConfigSystemShutdown();
}

bool Config::ConfigSystemStartup(const std::string& configFile)
{
	return ConfigLoadFile(configFile);
}

void Config::ConfigSystemShutdown()
{

}

void Config::ConfigSetString(const std::string& id, const std::string& value)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
			iterate->second = value;
	}
}

void Config::ConfigSetInt(const std::string& id, const int value)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
			iterate->second = std::to_string(value);
	}
}

void Config::ConfigSetFloat(const std::string& id, const float value)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
			iterate->second = std::to_string(value);
	}
}

void Config::ConfigSetBool(const std::string& id, const bool value)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id && value == true)
			iterate->second = "1";

		if (iterate->first == id && value == false)
			iterate->second = "0";
	}
}

void Config::ConfigSetRGBA(const std::string& id, const Rgba& color)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			std::string stringValue = std::to_string(color.r) + "," + std::to_string(color.g) + "," + std::to_string(color.b) + "," + std::to_string(color.a);
			iterate->second = stringValue;
		}
	}
}

bool Config::IsConfigSet(const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			return true;
		}
	}
	return false;
}

void Config::ConfigUnset(const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			m_configs.erase(iterate);
		}
	}
}

bool Config::ConfigGetString(std::string& outv, const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			outv = iterate->second;
			return true;
		}
	}
	return false;
}

bool Config::ConfigGetBool(bool& outv, const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id) 
		{
			if (iterate->second == "0")
			{
				outv = false;
				return true;
			}
			else
			{
				outv = true;
				return true;
			}
		}
	}
	return false;
}

bool Config::ConfigGetInt(int& outv, const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			outv = std::stoi(iterate->second);
			return true;
		}
	}
	return false;
}

bool Config::ConfigGetUInt(unsigned int& outv, const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			outv = static_cast<unsigned int>(std::stoul(iterate->second));
			return true;
		}
	}
	return false;
}

bool Config::ConfigGetFloat(float& outv, const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			outv = std::stof(iterate->second);
			return true;
		}
	}
	return false;
}

bool Config::ConfigGetNetAddress(net_address_t& out_addr, const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			out_addr = StringToNetAddress(iterate->second);
			return true;
		}
	}
	return false;
}

bool Config::ConfigGetRGBA(Rgba& outv, const std::string& id)
{
	for (auto iterate = m_configs.begin(); iterate != m_configs.end(); ++iterate)
	{
		if (iterate->first == id)
		{
			std::stringstream stream(iterate->second);
			std::string stringValue;
			std::vector<float> floatValues;

			while (std::getline(stream, stringValue, ','))
			{
				floatValues.push_back(std::stof(stringValue));
			}

			Rgba converter;
			outv.r = converter.ConvertFloatColorToByteColor(floatValues[0]);
			outv.g = converter.ConvertFloatColorToByteColor(floatValues[1]);
			outv.b = converter.ConvertFloatColorToByteColor(floatValues[2]);
			outv.a = converter.ConvertFloatColorToByteColor(floatValues[3]);
			return true;
		}
	}
	return false;
}

void Config::ConfigList()
{
	for (auto iter = m_configs.begin(); iter != m_configs.end(); ++iter)
	{
		auto currentKeyValue = *iter;
		std::string key = currentKeyValue.first;
		std::string value = currentKeyValue.second;
		std::ostringstream ss;
		ss << key << " = " << value << '\n';
		DebuggerPrintf(ss.str().c_str());
		ss.str("");
	}
}

bool Config::ConfigLoadFile(const std::string& filename)
{
	bool test = true;

	std::vector<unsigned char> buffer;
	test &= LoadBinaryFileToBuffer(filename, buffer);

	if (!test)
		return false;

	std::string key;

	for (unsigned int bufferIndex = 0; test && (bufferIndex < buffer.size()); )
	{
		test &= CharacterStateCheck(bufferIndex, buffer, key);
	}

	return test;
}

bool Config::CharacterStateCheck(unsigned int& bufferIndex, const std::vector<unsigned char>& buffer, std::string& key)
{
	if (bufferIndex == (buffer.size() - 1) || bufferIndex == (buffer.size() - 2))
	{
		++bufferIndex;
		return true;
	}

	if (!key.empty())
	{
		skipEqualSign(bufferIndex, buffer);
		std::string value = getValueFromBuffer(bufferIndex, buffer);
		convertTextBoolToBool(value);
		m_configs.insert(std::pair<std::string, std::string>(key,value));
		
		key.clear();
		return true;
	}
	
	const unsigned char character = buffer[bufferIndex];

	if (character == '#')
	{
		skipComment(bufferIndex, buffer);
		return true;
	}

	if (character == '<')
		++bufferIndex;

	if (character == '>')
		++bufferIndex;

	if (character == '+')
	{
		++bufferIndex;
		std::string value = getValueFromBuffer(bufferIndex, buffer);
		m_configs.insert(std::pair<std::string, std::string>(value, "1"));
		return true;
	}

	if (character == '-')
	{
		++bufferIndex;
		std::string value = getValueFromBuffer(bufferIndex, buffer);
		m_configs.insert(std::pair<std::string, std::string>(value, "0"));
		return true;
	}

	if (isWhiteSpace(bufferIndex, buffer))
	{
		skipWhiteSpace(bufferIndex, buffer);
		return true;
	}

	while (bufferIndex < buffer.size() && !isWhiteSpace(bufferIndex, buffer)) 
	{
		if(buffer[bufferIndex] != '=')
		{
			key += buffer[bufferIndex];
			++bufferIndex;
		}
		else
		{
			++bufferIndex;
			break;
		}
	}

	return true;
}

void Config::skipEqualSign(unsigned int& index, const std::vector<unsigned char>& buffer)
{
	skipWhiteSpace(index, buffer);

	if (buffer[index] == '=')
	{
		++index;
	}
}

void Config::skipComment(unsigned int& index, const std::vector<unsigned char>& buffer)
{
	while (index < buffer.size() && !isEndLine(index, buffer))
	{
		++index;
	}

	while (index < buffer.size() && isEndLine(index, buffer))
	{
		++index;
	}
}

void Config::skipWhiteSpace(unsigned int& index, const std::vector<unsigned char>& buffer)
{
	while (index < buffer.size() && isWhiteSpace(index, buffer))
	{
		++index;
	}
}

bool Config::isWhiteSpace(unsigned int& index, const std::vector<unsigned char>& buffer)
{
	if (buffer[index] == ' ')
		return true;

	return isEndLine(index, buffer);
}

bool Config::isEndLine(unsigned int& index, const std::vector<unsigned char>& buffer)
{
	if (buffer[index] == '\r')
		return true;

	if (buffer[index] == '\n')
		return true;

	return false;
}

std::string Config::getValueFromBuffer(unsigned int& index, const std::vector<unsigned char>& buffer)
{
	skipWhiteSpace(index, buffer);
	if (buffer[index] == '"')
		return getQuote(index, buffer);
	else
	{
		std::string value;
		while (index < buffer.size() && !isWhiteSpace(index, buffer))
		{
			value += buffer[index];
			++index;
		}
		return value;
	}
}

std::string Config::getQuote(unsigned int& index, const std::vector<unsigned char>& buffer)
{
	++index;
	std::string value;
	while (index < buffer.size() && buffer[index] != '"')
	{
		value += buffer[index];
		++index;
	}
	++index;
	return value;
}

void Config::convertTextBoolToBool(std::string& value)
{
	std::string test = value;
	std::transform(value.begin(), value.end(), test.begin(), std::tolower);

	if (test == "true")
	{
		value = "1";
		return;
	}

	if (test == "false")
	{
		value = "0";
		return;
	}
}