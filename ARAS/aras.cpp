#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <WinSock2.h>
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdio>

#include "Aras.h"

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

	m_newVersion = newVersionAvailable(m_setupUrl, m_msiUrl);
	if (m_newVersion) {
		// Create pop-up to ask if the user wants to install the new version
		MSGBOXPARAMSA msgboxParams = { sizeof(msgboxParams) };
		msgboxParams.hwndOwner = NULL;
		msgboxParams.hInstance = NULL;
		msgboxParams.lpszText = "A new version of ARAS is available. Do you want to install it now?";
		msgboxParams.lpszCaption = "ARAS Updater";
		msgboxParams.dwStyle = MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2;
		msgboxParams.dwContextHelpId = 0;
		int msgboxResult = MessageBoxIndirectA(&msgboxParams);
		if (msgboxResult == IDYES) {
			std::cerr << "User chose to install the new version now." << std::endl;
			m_loadingWindow = std::make_unique<GuiLoadingWindow>(500, 220, "Updater", this);
			if (m_loadingWindow->createWindow()) {
				if (m_loadingWindow->isOpen()) {
					m_loadingWindow->render();
				}
			}
			else {
				std::cerr << "Failed to create Loading window." << std::endl;
			}
			downloadFiles(m_setupUrl, m_msiUrl);
			std::cerr << "Installer files downloaded." << std::endl;
			launchInstaller();
		}
		std::cerr << "User chose not to install the new version now." << std::endl;
	}
	else {
		// remove any existing installer files
		if (std::filesystem::exists("ARASsetup.exe")) {
			if (std::remove("ARASsetup.exe") != 0) std::cerr << "Error deleting file" << std::endl;
		}
		if (std::filesystem::exists("ARASsetup.msi")) {
			if (std::remove("ARASsetup.msi") != 0) std::cerr << "Error deleting file" << std::endl;
		}
	}

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

bool Aras::newVersionAvailable(std::string& setupUrl, std::string& msiUrl)
{
	httplib::Client cli("https://api.github.com");
	httplib::Headers headers = { {"User-Agent", "ARAS-Updater"} };
	std::string apiEndpoint = "/repos/AlexisBalzano/ARASv2/releases/latest";

	auto res = cli.Get(apiEndpoint.c_str(), headers);
	if (res && res->status == 200) {
		auto json = nlohmann::json::parse(res->body);
		std::string latestVersion = json["tag_name"];
		if (json["assets"].empty()) {
			std::cerr << "No assets found for the latest release." << std::endl;
			return false;
		}
		if (latestVersion != ARAS_VERSION) {
			std::cerr << "A new version of ARAS is available: " << latestVersion << std::endl;

			for (auto& asset : json["assets"]) {
				std::string name = asset["name"];
				if (name == "ARASsetup.exe") {
					setupUrl = asset["browser_download_url"];
				}
				else if (name == "ARASsetup.msi") {
					msiUrl = asset["browser_download_url"];
				}
			}

			return true;
		}
		else {
			std::cerr << "You are using the latest version of ARAS." << std::endl;
			return false;
		}
	}
	else {
		std::cerr << "Error parsing new version: " << res.error() << std::endl;
		return false;
	}
}

bool Aras::downloadInstaller(std::ofstream& out, const std::string& url)
{
	if (!out) {
		std::cerr << "Failed to open file for writing" << std::endl;
		return false;
	}

	if (url.empty()) {
		std::cerr << "No URL provided for download" << std::endl;
		return false;
	}

	httplib::Client cli("https://github.com");
	cli.set_follow_location(true);
	httplib::Headers headers = { {"User-Agent", "ARAS-Updater"} };

	auto res = cli.Get(url.substr(std::string("https://github.com").size()).c_str(),
		headers,
		[&](const char* data, size_t data_length) {
			out.write(data, data_length);
			return true;  // keep receiving
		},
		[&](uint64_t current, uint64_t total) {
			if (total > 0) {
				int percent = static_cast<int>((current * 100) / total);
				m_loadingWindow->setProgress(static_cast<float>(percent));
				m_loadingWindow->render();
				std::cout << "\rDownloading... " << percent << "% ("
					<< current / 1024 << " KB / "
					<< total / 1024 << " KB)" << std::flush;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
			else {
				std::cout << "\rDownloading... " << current / 1024 << " KB" << std::flush;
			}
			return true; // continue download
		});

	if (res && res->status == 200) {
		std::cout << "Download complete!" << std::endl;
		return true;
	}
	else {
		std::cerr << "Download failed: " << (res ? std::to_string(res->status) : httplib::to_string(res.error())) << std::endl;
		return false;
	}
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

void Aras::downloadFiles(const std::string& setupUrl, const std::string& msiUrl)
{
	std::ofstream setupOut("ARASsetup.exe", std::ios::binary);
	m_loadingWindow->setText("ARASsetup.exe");
	bool success = downloadInstaller(setupOut, setupUrl);
	setupOut.close();
	if (!success) {
		if (std::remove("ARASsetup.exe") != 0) std::cerr << "Error deleting file" << std::endl;
	}

	std::ofstream msiOut("ARASsetup.msi", std::ios::binary);
	m_loadingWindow->setText("ARASsetup.msi");
	success = downloadInstaller(msiOut, msiUrl);
	msiOut.close();
	if (!success) {
		if (std::remove("ARASsetup.msi") != 0) std::cerr << "Error deleting file" << std::endl;
	}
}

void Aras::launchInstaller()
{
	SHELLEXECUTEINFOA shEx = { sizeof(shEx) };
	shEx.fMask = SEE_MASK_NOCLOSEPROCESS;
	shEx.hwnd = NULL;
	shEx.lpVerb = "runas";
	shEx.lpFile = "ARASsetup.exe";
	shEx.lpParameters = NULL;
	shEx.lpDirectory = NULL;
	shEx.nShow = SW_SHOWNORMAL;

	if (!ShellExecuteExA(&shEx)) {
		std::cerr << "Failed to launch installer, error: " << GetLastError() << std::endl;
		return;
	}

	CloseHandle(shEx.hProcess);

	std::cerr << "Installer launched, exiting ARAS." << std::endl;
	ExitProcess(0);
}

RunwayData Aras::assignAirportRunway(const std::string& airport, const WindData& windData)
{
	// Add connected airports logic
	constexpr double PI = 3.14159265358979323846;
	std::vector<RunwayData> runwaysData = m_dataManager->getAirportRunwaysData(airport);

	int alpha = std::abs(windData.windDirection - runwaysData[0].heading);
	if (alpha > 180) alpha = 360 - alpha;

	double tailWindComponent = windData.windSpeed * std::cos(alpha * PI / 180.0) < 0 ? windData.windSpeed * std::cos(alpha * PI / 180.0) : 0;

	if (tailWindComponent < -runwaysData[0].preferential) { // Assume that the preferential runway is always the first one in the json
		return runwaysData[1];
	}
	else {
		return runwaysData[0];
	}
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
