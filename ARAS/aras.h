#pragma once
#include <vector>
#include <Thread>

#include "GuiWindow.h"
#include "DataManager.h"
#include "SoundSystem.h"

constexpr const char* ARAS_VERSION = "v1.0.0";

struct RunwayData {
	std::string airport;
	bool has4rwys = false;
	std::string depRunway;
	std::string arrRunway;
	std::string depRunwayBis;
	std::string arrRunwayBis;
	int heading = 0;
	int preferential = 0;
};

class Aras {
public:
	Aras();
	~Aras();

	// Lifecycle
	void initialise();
	void run();
	void shutdown();

	void createMainWindow();

	std::vector<std::string> getFIRs() const;
	std::vector<std::string> getAirports(const std::string& fir) const;
	std::vector<std::string> getDefaultAirports(const std::string& fir) const;
	std::filesystem::path getRwyFilePath() const { return m_dataManager->getRwyFilePath(); }
	std::string getTokenConfig() const { return m_dataManager->getToken(); }
	bool getTokenValidity() const { return m_dataManager->isTokenValid(); }

	bool isRwyFileFound() const { return std::filesystem::exists(m_dataManager->getConfigPath() / "rwydata.json"); }
	bool isConfigFileFound() const { return std::filesystem::exists(m_dataManager->getConfigPath() / "config.json"); }

	void assignRunways(const std::string& fir);
	void openSettings();
	void resetAirportsList();
	void saveToken(const std::string& token);
	void updateAirportsList(std::string fir, std::string airports);
	void saveRwyLocation(const std::filesystem::path path);
	void addFIR(const std::string& fir);

	RunwayData assignAirportRunway(const std::string& airport, const WindData& windData);
	std::vector<std::string> formatRunwayOutput(const RunwayData& runwayData);
	std::vector<std::string> formatActiveAirport(const std::string& airport);

private:
	std::unique_ptr<DataManager> m_dataManager;
	std::unique_ptr<SoundPlayer> m_soundPlayer;
	std::thread m_renderThread;
	bool m_stop = false;

	std::vector<std::unique_ptr<GuiWindow>> m_windows;
	std::vector<std::unique_ptr<GuiWindow>> newWindows;
	
};