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


private:
	std::thread m_renderThread;
	bool m_stop = false;

	std::vector<std::unique_ptr<GuiWindow>> m_windows;
	
};