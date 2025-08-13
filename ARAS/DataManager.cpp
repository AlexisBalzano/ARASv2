#include "DataManager.h"
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <cstdlib>
#endif

DataManager::DataManager()
{
	m_configPath = GetConfigPath();
	parseConfigFile(); // returns true if successful can be use to create default config if it doesn't exist
}

std::filesystem::path DataManager::GetConfigPath()
{
#if defined(_WIN32)
	PWSTR path = nullptr;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
	std::filesystem::path documentsPath;
	if (SUCCEEDED(hr)) {
		documentsPath = path;
		CoTaskMemFree(path);
	}
	return documentsPath / "Aras";
#elif defined(__APPLE__) || defined(__linux__)
	const char* homeDir = std::getenv("HOME");
	if (homeDir) {
		return std::filesystem::path(homeDir) / "Documents" / "Aras";
	}
	return std::filesystem::path(); // Return empty path if HOME is not set
#else
	return std::filesystem::path(); // Return an empty path for unsupported platforms
#endif
}

bool DataManager::parseConfigFile()
{

	std::ifstream configFile(m_configPath / "config.json");
	if (!configFile.is_open()) {
		std::cout << "Failed to open config file." << std::endl;
		return false;
	}
	try {
		configFile >> m_configJson;
		configFile.close();
		return true;
	}
	catch (const std::exception& e) {
		std::cout << "Error parsing config file: " << e.what() << std::endl;
		return false;
	}
}
