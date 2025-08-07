#include "aras.h"
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

	createMainWindow();

	while (true) {
		if (m_stop)
			return;

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		for (auto& window : m_windows) {
			while (const std::optional<sf::Event> event = window->pollWindowEvent()) {
				window->processEvents(*event);
			}
			if (window->isOpen()) {
				window->render();
			}
			else {
				return;
			}
		}
	}

	shutdown();
}

void Aras::run()
{
	//If you want to run the main loop in a separate thread, you can implement it here.
}

void Aras::shutdown()
{
	m_stop = true;
	//if (m_renderThread.joinable())
		//m_renderThread.join();
}

void Aras::createMainWindow()
{
	std::unique_ptr<GuiWindow> mainWindow = std::make_unique<GuiWindow>(1000, 500, "ARAS");
	if (mainWindow->createWindow()) {
		m_windows.push_back(std::move(mainWindow));
	} else {
		std::cerr << "Failed to create main window." << std::endl;
	}
}
