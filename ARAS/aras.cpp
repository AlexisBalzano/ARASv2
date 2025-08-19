#include "Aras.h"
#include <iostream>

Aras::Aras()
{
	initialise();
}

Aras::~Aras()
{
	shutdown();
}

void Aras::initialise()
{
	m_stop = false;
	m_windows.clear();
	//m_renderThread = std::thread(&Aras::run, this);

	m_dataManager = std::make_unique<DataManager>();
	m_soundPlayer = std::make_unique<SoundPlayer>();

	createMainWindow();

	run();
	
	shutdown();
}

void Aras::run()
{
	while (true) {
		if (m_stop)
			return;

		if (m_windows.empty()) return;

		for (auto& window : m_windows) {
			while (const std::optional<sf::Event> event = window->pollWindowEvent()) {
				window->processEvents(*event);
			}
			if (window->isOpen()) {
				window->render();
			}
		}

		m_windows.erase(std::remove_if(m_windows.begin(), m_windows.end(),
			[](const std::unique_ptr<GuiWindow>& window) {
				return !window->isOpen();
			}), m_windows.end()
				);

		for (auto& win : newWindows) {
			m_windows.push_back(std::move(win));
		}
		newWindows.clear();
	}
}

void Aras::shutdown()
{
	m_stop = true;
	//if (m_renderThread.joinable())
		//m_renderThread.join();
}

void Aras::createMainWindow()
{
	std::unique_ptr<GuiWindow> mainWindow = std::make_unique<GuiMainWindow>(1000, 500, "ARAS", this);
	if (mainWindow->createWindow()) {
		m_windows.push_back(std::move(mainWindow));
	} else {
		std::cerr << "Failed to create main window." << std::endl;
	}
}

std::vector<std::string> Aras::getFIRs() const
{
	return m_dataManager->getFIRs();
}

std::vector<std::string> Aras::getAirports(const std::string& fir) const
{
	return m_dataManager->getAirportsList(fir);
}

std::vector<std::string> Aras::getDefaultAirports(const std::string& fir) const
{
	return m_dataManager->getDefaultAirportsList(fir);
}

void Aras::assignRunways(const std::string& fir)
{
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	
	std::vector<std::string> airports = m_dataManager->getAirportsList(fir);
	if (airports.empty()) {
		std::cout << "No airports found for FIR: " << fir << std::endl;
		return;
	}

	std::vector<std::string> runwayText;
	std::vector<std::future<WindData>> windDataFutureList = m_dataManager->getWindData(airports);
	
	for (size_t i = 0; i < airports.size(); ++i) {
		std::cout << "Processing airport: " << airports[i] << std::endl;
		WindData windData = windDataFutureList[i].get(); // Wait for wind data to be ready
		if (windData.windDirection == -1 || windData.windSpeed == -1) {
			std::cout << "Invalid wind data for airport: " << airports[i] << std::endl;
			continue;
		}

		std::vector<std::string> activeAirportStrings = formatActiveAirport(airports[i]);
		runwayText.insert(runwayText.end(), activeAirportStrings.begin(), activeAirportStrings.end());

		std::vector<std::string> assignedRunwayStrings = formatRunwayOutput(assignAirportRunway(airports[i], windData));
		runwayText.insert(runwayText.end(), assignedRunwayStrings.begin(), assignedRunwayStrings.end());
	}

	m_dataManager->outputRunways(runwayText);

	m_soundPlayer->playSound(SoundPlayer::completionSound);
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "Runway assignment completed in " << elapsed_seconds.count() << " seconds." << std::endl;
}

void Aras::openSettings()
{
	for (const auto& window : m_windows) {
		if (window->getTitle() == "Settings") {
			// Bring the existing settings window to the front
			window->focus();
			return;
		}
	}
	std::unique_ptr<GuiWindow> settingWindow = std::make_unique<GuiSettingWindow>(500, 500, "Settings", this);
	if (settingWindow->createWindow()) {
		newWindows.push_back(std::move(settingWindow));
	}
	else {
		std::cerr << "Failed to create Setting window." << std::endl;
	}
}

void Aras::resetAirportsList()
{
	std::cout << "Resetting airports list..." << std::endl;
}

void Aras::saveToken(const std::string& token)
{
	m_dataManager->updateToken(token);
}

void Aras::updateAirportsList(std::string fir, std::string airports)
{
	m_dataManager->updateAirportsConfig(fir, airports);
}

void Aras::saveRwyLocation(const std::filesystem::path path)
{
	m_dataManager->updateRwyLocation(path);
}

void Aras::addFIR(const std::string& fir)
{
	m_dataManager->addFIRconfig(fir);
}

RunwayData Aras::assignAirportRunway(const std::string& airport, const WindData& windData)
{
	std::vector<RunwayData> runwaysData = m_dataManager->getAirportRunwaysData(airport);

	//TODO: Calculate the best runway based on wind data
	size_t i = 0;
	return runwaysData[i]; // Return selected runway's RunwayData
}

std::vector<std::string> Aras::formatRunwayOutput(const RunwayData& runwaysData)
{
	std::string standardOutput = "ACTIVE_RUNWAY:" + runwaysData.airport + ":";
	std::vector<std::string> output;

	output.emplace_back(standardOutput + runwaysData.depRunway + ":1");
	output.emplace_back(standardOutput + runwaysData.arrRunway + ":0");

	if (runwaysData.has4rwys) {
		output.emplace_back(standardOutput + runwaysData.depRunwayBis + ":1");
		output.emplace_back(standardOutput + runwaysData.arrRunwayBis + ":0");
	}

	return output;
}

std::vector<std::string> Aras::formatActiveAirport(const std::string& airport)
{
	std::string activeAirportText = "ACTIVE_AIRPORT:" + airport + ":";
	return std::vector<std::string>{activeAirportText + "1", activeAirportText + "0"};
}
