#pragma once
#include <iostream>
#include <fstream>
#include "Aras.h"

bool newVersionAvailable(std::string& downloadUrl) {
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
		downloadUrl = json["assets"][0]["browser_download_url"];
		if (latestVersion != ARAS_VERSION) {
			std::cerr << "A new version of ARAS is available: " << latestVersion << std::endl;
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

void installNewVersion(const std::string& url) {
	std::ofstream out("ARAS-Installer.exe", std::ios::binary);
	if (!out) {
		std::cerr << "Failed to open file for writing" << std::endl;
		return;
	}

	httplib::Client cli("https://github.com");
	cli.set_follow_location(true);
	httplib::Headers headers = { {"User-Agent", "ARAS-Updater"} };

	auto res = cli.Get(url.substr(std::string("https://github.com").size()).c_str(),
		headers,
		[&](const char* data, size_t data_length) {
			out.write(data, data_length);
			return true;  // keep receiving
		});

	if (res && res->status == 200) {
		std::cout << "Download complete!" << std::endl;
	}
	else {
		std::cerr << "Download failed: "
			<< (res ? std::to_string(res->status) : httplib::to_string(res.error()))
			<< std::endl;
	}
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
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

	std::string downloadUrl;
	bool newVersion = newVersionAvailable(downloadUrl);

	if (newVersion) {
		installNewVersion(downloadUrl);
	}

	Aras aras;

	std::cerr << "Closing log file." << std::endl;
    return 0;
}