#pragma once
#include <string>
#include <filesystem>
#include <future>
#include <nlohmann/json.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

class DataManager {
public:
	DataManager();
	~DataManager();
	
	std::filesystem::path getConfigPath();
	bool parseConfigFile();
	void createDefaultConfig();
	bool outputConfig();
	bool outputRunways(const std::vector<std::string> runways);

	void updateAirportsConfig(const std::string& fir, std::string airports);
	void updateToken(const std::string& token);
	void updateRwyLocation(const std::filesystem::path& path);
	void addFIRconfig(const std::string& fir);

	bool isTokenValid() const { return m_configJson.value("tokenValidity", false); }

	std::vector<std::string> getAirportsList(const std::string& fir) const;
	std::vector<std::string> getDefaultAirportsList(const std::string& fir) const;
	std::vector<std::string> getFIRs() const;
	std::string getToken() const { return m_token; }
	std::filesystem::path getRwyFilePath() const { return m_rwyFilePath; }
	std::future<void> getHTTPSresponseAsync(const std::string& oaci);
	
private:
	std::filesystem::path m_configPath;

	nlohmann::json m_configJson;
	std::string m_token;
	std::filesystem::path m_rwyFilePath;
};