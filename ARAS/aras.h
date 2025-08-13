#pragma once
#include <vector>
#include <Thread>


#include "guiWindow.h"


class Aras {
public:
	Aras();
	~Aras();

	// Lifecycle
	void initialise();
	void run();
	void shutdown();

	void createMainWindow();

	void assignRunways();
	void openSettings();
	void resetAirportsList();
	void saveToken();
	void setStatus(); // Change return type accordingly


private:
	std::thread m_renderThread;
	bool m_stop = false;

	std::vector<std::unique_ptr<GuiWindow>> m_windows;
	std::vector<std::unique_ptr<GuiWindow>> newWindow;
	
};