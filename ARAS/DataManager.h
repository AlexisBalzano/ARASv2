#pragma once
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

class DataManager {
public:
	DataManager();
	~DataManager() = default;
	
	std::filesystem::path GetConfigPath();
	bool parseConfigFile();
	
private:
	std::filesystem::path m_configPath;

	nlohmann::json m_configJson;
};