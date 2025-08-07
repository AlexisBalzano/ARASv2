#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>


class GuiWindow {
public:
	GuiWindow(unsigned int width, unsigned int height, const std::string& title);
	~GuiWindow();

	GuiWindow(const GuiWindow&) = delete;
	GuiWindow& operator=(const GuiWindow&) = delete;

	GuiWindow(GuiWindow&&) noexcept = default;
	GuiWindow& operator=(GuiWindow&&) noexcept = default;

	bool createWindow();
	std::optional<sf::Event> pollWindowEvent();
	void processEvents(const sf::Event& event);
	void render();

	bool isOpen() const;



private:
	unsigned int m_width;
	unsigned int m_height;
	std::string m_title;
	sf::RenderWindow m_window;
	tgui::Gui m_gui;

	sf::Font m_font;
};