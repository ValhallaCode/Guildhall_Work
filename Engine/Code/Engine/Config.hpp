#pragma once
#include "Engine/Network/NetAddress.hpp"
#include <map>
#include <vector>


class Rgba;

class Config
{
public:
	Config(const std::string &filePath);
	~Config();

	// Startup Config System - Setup Initial Configs by parsing supplied file.
	bool ConfigSystemStartup(const std::string& configFile);
	void ConfigSystemShutdown();

	// Setters - feel free to use std::string
	void ConfigSetString(const std::string& id, const std::string& value);
	void ConfigSetInt(const std::string& id, const int value);
	void ConfigSetFloat(const std::string& id, const float value);
	void ConfigSetBool(const std::string& id, const bool value);
	void ConfigSetRGBA(const std::string& id, const Rgba& color);

	// Easy check for simple on/off flags. 
	bool IsConfigSet(const std::string& id);
	void ConfigUnset(const std::string& id);

	// Fetching.  If a config exists and is 
	// convertible to the desired type, 
	// place its converted value in the out variable,
	// and return true;
	// Otherwise, return false.
	bool ConfigGetString(std::string& outv, const std::string& id);
	bool ConfigGetBool(bool& outv, const std::string& id);
	bool ConfigGetInt(int& outv, const std::string& id);
	bool ConfigGetUInt(unsigned int& outv, const std::string& id);
	bool ConfigGetFloat(float& outv, const std::string& id);
	bool ConfigGetNetAddress(net_address_t& out_addr, const std::string& id);
	bool ConfigGetRGBA(Rgba& outv, const std::string& id);

	// [DEBUG FEATURE] List all currently existing configs.
	void ConfigList();

	// Load a config file
	// see sample.config 
	bool ConfigLoadFile(const std::string& filename);
	bool CharacterStateCheck(unsigned int& bufferIndex, const std::vector<unsigned char>& buffer, std::string& key);
	void skipEqualSign(unsigned int& index, const std::vector<unsigned char>& buffer);
	void skipWhiteSpace(unsigned int& index, const std::vector<unsigned char>& buffer);
	bool isWhiteSpace(unsigned int& index, const std::vector<unsigned char>& buffer);
	bool isEndLine(unsigned int& index, const std::vector<unsigned char>& buffer);
	std::string getValueFromBuffer(unsigned int& index, const std::vector<unsigned char>& buffer);
	void convertTextBoolToBool(std::string& value);
	std::string getQuote(unsigned int& index, const std::vector<unsigned char>& buffer);
	void Config::skipComment(unsigned int& index, const std::vector<unsigned char>& buffer);

	std::multimap<std::string, std::string> m_configs;
};

extern Config* g_config;