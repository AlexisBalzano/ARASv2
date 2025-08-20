#include "DataManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <cstdlib>
#endif

#include "Aras.h"

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
	}
	catch (const std::exception& e) {
		std::cout << "Error parsing config file: " << e.what() << std::endl;
		return false;
	}

	std::ifstream rwyDataFile(m_configPath / "rwydata.json");
	if (!rwyDataFile.is_open()) {
		std::cout << "Failed to open rwyData file." << std::endl;
		return false;
	}
	try {
		rwyDataFile >> m_rwyDataJson;
		rwyDataFile.close();
		return true;
	}
	catch (const std::exception& e) {
		std::cout << "Error parsing rwyData file: " << e.what() << std::endl;
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
			rwyFile << runway + "\n";
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
		return;
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
	m_configJson["FIR"][firUpper + "def"] = nlohmann::json::array();

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

std::future<WindData> DataManager::getWindData(const std::string& oaci)
{
	return std::async(std::launch::async, [this, oaci]() {
		httplib::Client cli("https://avwx.rest");
		httplib::Headers headers = {
			{"Authorization", "BEARER " + m_token}
		};
		std::string apiEndpoint = "/api/metar/"; // Example endpoint
		auto res = cli.Get(apiEndpoint + oaci, headers);
		if (res) {
			if (res->status == 200) {
				if (!m_configJson["tokenValidity"].get<bool>()) {
					m_configJson["tokenValidity"] = true;
					if (!outputConfig()) {
						std::cerr << "Failed to update token validity in config file." << std::endl;
					}
				}

				nlohmann::json responseJson;
				try {
					WindData windData{};
					responseJson = nlohmann::json::parse(res->body);
					windData.windDirection = responseJson["wind_direction"]["value"].is_null() ? 0 : responseJson["wind_direction"]["value"].get<int>();
					windData.windSpeed = responseJson["wind_speed"].value("value", 0);
					windData.windGust = responseJson["wind_gust"].is_null() ? 0 : responseJson["wind_gust"].value("value", 0);

					return windData;
				}
				catch (const std::exception& e) {
					std::cerr << "Error when parsing response: " << e.what() << std::endl;
				}
			}
		}
		return WindData{ -1, -1, -1 }; // Return invalid values if request fails
	});
}

std::vector<std::future<WindData>> DataManager::getWindData(const std::vector<std::string>& airports)
{
	std::vector<std::future<WindData>> windDataFutures;
	for (const auto& airport : airports) {
		if (!airport.empty()) {
			windDataFutures.push_back(getWindData(airport));
		}
	}
	return windDataFutures;
}

std::vector<RunwayData> DataManager::getAirportRunwaysData(const std::string& airport)
{
	std::vector<RunwayData> runwaysData;
	RunwayData runwayData;
	runwayData.airport = airport;

	if (m_rwyDataJson[airport].contains("has4runways")) {
		runwayData.has4rwys = true;
	}

	auto iterator = m_rwyDataJson[airport]["runways"].begin();
	while (iterator != m_rwyDataJson[airport]["runways"].end()) {
		std::string variant = iterator.key();
		nlohmann::json rwydataJson = m_rwyDataJson[airport]["runways"][variant];

		runwayData.depRunway = rwydataJson["departure"];
		runwayData.arrRunway = rwydataJson["arrival"];
		runwayData.heading = rwydataJson["heading"];
		runwayData.preferential = rwydataJson["preferential"];
		if (runwayData.has4rwys) {
			runwayData.depRunwayBis = rwydataJson["departureBis"];
			runwayData.arrRunwayBis = rwydataJson["arrivalBis"];
		}
		else {
			runwayData.depRunwayBis = "";
			runwayData.arrRunwayBis = "";
		}
		runwaysData.push_back(runwayData);
		
		++iterator;
	}

	return runwaysData;
}
