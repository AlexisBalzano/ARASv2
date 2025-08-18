#include "DataManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <cstdlib>
#include "Aras.h"
#endif

static std::string trim(const std::string& s) {
	auto start = s.begin();
	while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) ++start;
	auto end = s.end();
	do { --end; } while (end >= start && std::isspace(static_cast<unsigned char>(*end)));
	return std::string(start, end + 1);
}


DataManager::DataManager()
{
	m_configPath = getConfigPath();
	if (!parseConfigFile()) {
		createDefaultConfig();
	}
}

DataManager::~DataManager()
{
	outputConfig();
}

std::filesystem::path DataManager::getConfigPath()
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
		m_token = m_configJson.value("apitoken", "");
		if (m_configJson.contains("outputPath")) {
			if (!m_configJson["outputPath"].get<std::string>().empty()) {
				m_rwyFilePath = m_configJson["outputPath"].get<std::filesystem::path>();
			}
		}
		return true;
	}
	catch (const std::exception& e) {
		std::cout << "Error parsing config file: " << e.what() << std::endl;
		return false;
	}
}

void DataManager::createDefaultConfig()
{
	if (!std::filesystem::exists(m_configPath)) {
		std::filesystem::create_directories(m_configPath);
	}
	m_configJson = {
		{"apitoken", ""},
		{"tokenValidity", false},
		{"outputPath", ""},
		{"FIR", {}}
	};
	if (outputConfig()) {
		std::cout << "Default config created successfully." << std::endl;
	} else {
		std::cout << "Failed to create default config." << std::endl;
	}
}

bool DataManager::outputConfig()
{
	std::ofstream configFile(m_configPath / "config.json");
	if (!configFile.is_open()) {
		std::cout << "Failed to open config file for writing." << std::endl;
		return false;
	}
	try {
		configFile << m_configJson.dump(4);
		configFile.close();
		std::cout << "Config file written successfully." << std::endl;
		return true;
	}
	catch (const std::exception& e) {
		std::cout << "Error writing config file: " << e.what() << std::endl;
	}
	return false;
}

bool DataManager::outputRunways(const std::vector<std::string> runways)
{
	if (m_configJson.contains("outputPath")) {
		if (!m_configJson["outputPath"].is_null()) {
			m_rwyFilePath = m_configJson["outputPath"].get<std::filesystem::path>();
		}
		else {
			std::cout << "Output path not set" << std::endl;
			return false;
		}
	}
	std::ofstream rwyFile(m_rwyFilePath);
	if (!rwyFile.is_open()) {
		std::cout << "Failed to open runway file for writing." << std::endl;
		return false;
	}
	try {
		for (const auto& runway : runways) {
			if (runway.empty()) {
				std::cout << "Empty runway name found, skipping." << std::endl;
				continue;
			}
		}
		rwyFile.close();
		std::cout << "Runway file written successfully." << std::endl;
		return true;
	}
	catch (const std::exception& e) {
		std::cout << "Error writing runway file: " << e.what() << std::endl;
		return false;
	}
}

void DataManager::updateAirportsConfig(const std::string& fir, std::string airports)
{
	if (!m_configJson["FIR"].contains(fir)) {
		m_configJson["FIR"][fir] = nlohmann::json::array();
	}
	nlohmann::json& firAirports = m_configJson["FIR"][fir];
	std::vector<std::string> newAirports;

	std::istringstream ss(airports);
	std::string token;
	while (std::getline(ss, token, ',')) {
		std::string trimmed = trim(token);
		if (!trimmed.empty()) {
			std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), ::toupper);
			newAirports.push_back(trimmed);
		}
	}

	firAirports = newAirports;
	if (!outputConfig()) {
		std::cout << "Failed to update airports in config file." << std::endl;
	}
}

void DataManager::updateToken(const std::string& token)
{
	if (token.empty()) {
		return;
	}
	m_token = token;
	m_configJson["apitoken"] = m_token;
	m_configJson["tokenValidity"] = false;
	if (!outputConfig()) {
		std::cout << "Failed to update token in config file." << std::endl;
	}
}

void DataManager::updateRwyLocation(const std::filesystem::path& path)
{
	if (path.empty() || !std::filesystem::exists(path)) {
		std::cout << "Invalid runway location path." << std::endl;
		return;
	}
	m_rwyFilePath = path;
	m_configJson["outputPath"] = m_rwyFilePath.string();
	if (!outputConfig()) {
		std::cout << "Failed to update runway location in config file." << std::endl;
	}
}

void DataManager::addFIRconfig(const std::string& fir)
{
	if (fir.empty() || m_configJson["FIR"].contains(fir)) {
		return;
	}
	std::string firUpper = trim(fir);
	m_configJson["FIR"][firUpper] = nlohmann::json::array();
	if (!outputConfig()) {
		std::cout << "Failed to add FIR configuration." << std::endl;
	}
}

std::vector<std::string> DataManager::getAirportsList(const std::string& fir) const
{
	if (!m_configJson.contains("FIR")) return std::vector<std::string>();
	
	const nlohmann::json& firs = m_configJson["FIR"];
	
	if (firs.contains(fir)) {
		std::vector<std::string> airports;
		for (const auto& item : firs[fir]) {
			if (item.is_string()) {
				airports.push_back(item.get<std::string>());
			}
		}
		return airports;
	}
	return std::vector<std::string>();
}

std::vector<std::string> DataManager::getDefaultAirportsList(const std::string& fir) const
{
	return getAirportsList(fir + "def");
}

std::vector<std::string> DataManager::getFIRs() const
{
	std::vector<std::string> firs;
	if (m_configJson.contains("FIR") && m_configJson["FIR"].is_object()) {
		for (auto it = m_configJson["FIR"].begin(); it != m_configJson["FIR"].end(); ++it) {
			if (it.key().substr(it.key().size() - 3) == "def") {
				continue;
			}
			firs.push_back(it.key());
		}
	}
	return firs;
}
