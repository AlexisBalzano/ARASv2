#pragma once
#include <iostream>
#include <fstream>

#include "Aras.h"


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

	Aras aras;

	std::cerr << "Closing log file." << std::endl;
    return 0;
}