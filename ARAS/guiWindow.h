#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "Colors.h"

#ifdef _WIN32
#include <windows.h>
#endif


struct ButtonColors {
	tgui::Color background = Colors::Grey;
	tgui::Color backgroundDown = Colors::DarkGrey;
	tgui::Color backgroundHover = Colors::LightGrey;
	tgui::Color text = tgui::Color::White;
	tgui::Color textHover = tgui::Color::Black;
	tgui::Color textDown = tgui::Color::White;
};

class GuiWindow {
public:
	GuiWindow(unsigned int width, unsigned int height, const std::string& title);
	~GuiWindow();

	GuiWindow(const GuiWindow&) = delete;
	GuiWindow& operator=(const GuiWindow&) = delete;

	GuiWindow(GuiWindow&&) noexcept = default;
	GuiWindow& operator=(GuiWindow&&) noexcept = default;

	virtual bool createWindow();
	std::optional<sf::Event> pollWindowEvent();
	virtual void processEvents(const sf::Event& event);
	void render();

	bool isOpen() const;

#ifdef _WIN32
	void updateDrag();
#endif // _WIN32

protected:
	tgui::Button::Ptr createButton(const std::string& buttonText, tgui::Vector2f position, tgui::Vector2f size, ButtonColors colors);


protected:
	unsigned int m_width;
	unsigned int m_height;
	std::string m_title;
	sf::RenderWindow m_window;
	tgui::Gui m_gui;

#ifdef _WIN32
	HWND m_hwnd = nullptr; // Handle to the window for dragging
	bool m_dragging = false; // Flag to check if the window is being dragged
	POINT m_clickOffset; // Offset of the click position relative to the window position
#endif

	std::vector <std::unique_ptr<sf::Drawable>> m_drawables;

	sf::Font m_font;
};

class GuiMainWindow : public GuiWindow {
public:
	GuiMainWindow(unsigned int width, unsigned int height, const std::string& title);
	
	bool createWindow() override;

private:
	tgui::Button::Ptr m_closeButton;
	tgui::Button::Ptr m_minimiseButton;

	POINT clickOffset_{};
	
};