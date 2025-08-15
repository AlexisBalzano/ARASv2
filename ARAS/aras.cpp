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

void Aras::assignRunways()
{
	std::cout << "Assigning runways..." << std::endl;
	// Implement runway assignment logic here
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
