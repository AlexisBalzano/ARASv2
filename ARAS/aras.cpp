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
	std::cout << "Assigning runways fo FIR: " + fir << std::endl;
	std::vector<std::string> airports = m_dataManager->getAirportsList(fir);
	if (airports.empty()) {
		std::cout << "No airports found for FIR: " << fir << std::endl;
		return;
	}

	std::vector<std::future<WindData>> windDataFutureList;
	
	for (const auto& airport : airports) {
		windDataFutureList.emplace_back(m_dataManager->getWindData(airport));
	}
	
	for (size_t i = 0; i < airports.size(); ++i) {
		std::cout << "Processing airport: " << airports[i] << std::endl;
		WindData windData = windDataFutureList[i].get(); // Wait for wind data to be ready
		// process runway assignment logic here
		// Check for invalid wind data (-1)
		if (windData.windDirection == -1 || windData.windSpeed == -1) {
			std::cout << "Invalid wind data for airport: " << airports[i] << std::endl;
			continue; // Skip this airport
		}
		else {
			std::cout << "Wind data for " << airports[i] << ": " << "Direction: " << windData.windDirection << ", Speed: " << windData.windSpeed << ", Gust: " << windData.windGust << std::endl;
		}
	}

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	m_soundPlayer->playSound(Sounds::completionSound);
	std::cout << "Runway assignment completed in " << elapsed_seconds.count() << " seconds." << std::endl;

}

void Aras::openSettings()
{
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

void Aras::setStatus()
{
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
