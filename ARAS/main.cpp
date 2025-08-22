#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <WinSock2.h>
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "Aras.h"

static bool newVersionAvailable(std::string& setupUrl, std::string& msiUrl) {
    // Check for updates
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

static bool downloadInstaller(std::ofstream& out, const std::string& url) {
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
				std::cout << "\rDownloading... " << percent << "% ("
					<< current / 1024 << " KB / "
					<< total / 1024 << " KB)" << std::flush;
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

static void downloadFiles(const std::string& setupUrl, const std::string& msiUrl) {
	std::ofstream setupOut("ARASsetup.exe", std::ios::binary);
	bool success = downloadInstaller(setupOut, setupUrl);
	setupOut.close();
	if (!success) {
		if (std::remove("ARASsetup.exe") != 0) std::cerr << "Error deleting file" << std::endl;
	}

	std::ofstream msiOut("ARASsetup.msi", std::ios::binary);
	success = downloadInstaller(msiOut, msiUrl);
	msiOut.close();
	if (!success) {
		if (std::remove("ARASsetup.msi") != 0) std::cerr << "Error deleting file" << std::endl;
	}
}

static void launchInstaller() {
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


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
	// Initialize logging
	std::ofstream logFile("ARAS_log.txt");
	if (logFile.is_open()) {
		logFile << "----- ARAS Log Started -----" << std::endl;
		std::cerr.rdbuf(logFile.rdbuf());
		std::cout.rdbuf(logFile.rdbuf());
		std::cerr << "ARAS version: " << ARAS_VERSION << std::endl;
	}
	else {
		std::cerr << "Failed to open log file." << std::endl;
	}

	std::string setupUrl;
	std::string msiUrl;
	bool newVersion = newVersionAvailable(setupUrl, msiUrl);

	if (newVersion) {
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
			downloadFiles(setupUrl, msiUrl);
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


	Aras aras;

	std::cerr << "Closing log file." << std::endl;
    return 0;
}