#pragma once
#include <vector>
#include <Thread>


#include "GuiWindow.h"
#include "DataManager.h"


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
	std::string getTokenConfig() const { return m_dataManager->getToken(); }

	void assignRunways();
	void openSettings();
	void resetAirportsList();
	void saveToken(const std::string& token);
	void setStatus(); // Change return type accordingly
	void updateAirportsList(std::string fir, std::string airports);
	void saveRwyLocation(const std::filesystem::path path);
	std::filesystem::path getRwyFilePath() const { return m_dataManager->getRwyFilePath(); }


private:
	std::unique_ptr<DataManager> m_dataManager;
	std::thread m_renderThread;
	bool m_stop = false;

	std::vector<std::unique_ptr<GuiWindow>> m_windows;
	std::vector<std::unique_ptr<GuiWindow>> newWindows;
	
};