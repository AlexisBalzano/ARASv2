#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <string>

#include "Colors.h"
#include "RoundedRectangle.h"

#ifdef _WIN32
#include <windows.h>
#endif

class Aras;


struct ButtonColors {
	tgui::Color background = Colors::LightGrey;
	tgui::Color backgroundDown = Colors::DarkGrey;
	tgui::Color backgroundHover = Colors::Grey;
	tgui::Color text = tgui::Color::White;
	tgui::Color textHover = tgui::Color::Black;
	tgui::Color textDown = tgui::Color::White;
};

class GuiWindow {
public:
	GuiWindow(unsigned int width, unsigned int height, const std::string& title, Aras* aras);
	~GuiWindow();

	GuiWindow(const GuiWindow&) = delete;
	GuiWindow& operator=(const GuiWindow&) = delete;

	GuiWindow(GuiWindow&&) noexcept = default;
	GuiWindow& operator=(GuiWindow&&) noexcept = default;

	virtual bool createWindow();
	void createBaseWindowLayout(const std::string& title);
	std::optional<sf::Event> pollWindowEvent();
	virtual void processEvents(const sf::Event& event);
	void render();
	bool isOpen() const;
	std::string getTitle() const { return m_title; }

#ifdef _WIN32
	void updateDrag();
#endif // _WIN32

protected:
	tgui::Button::Ptr createButton(const std::string& buttonText, tgui::Vector2f position, tgui::Vector2f size, ButtonColors colors);


protected:
	Aras* m_aras = nullptr;
	unsigned int m_width;
	unsigned int m_height;
	std::string m_title;
	sf::RenderWindow m_window;
	tgui::Gui m_gui;
	tgui::CanvasSFML::Ptr backgroundCanvas;
	tgui::Button::Ptr m_closeButton;
	tgui::Button::Ptr m_minimiseButton;
	std::vector <std::unique_ptr<sf::Drawable>> m_drawables;
	sf::Texture iconTexture;
	sf::Image m_icon;
	sf::Font m_font;

#ifdef _WIN32
	HWND m_hwnd = nullptr;
	bool m_dragging = false;
	POINT m_clickOffset;
#endif

};

class GuiMainWindow : public GuiWindow {
public:
	GuiMainWindow(unsigned int width, unsigned int height, const std::string& title, Aras* aras);
	
	bool createWindow() override;
	void createMainWindowWidgets();
	void updateAirportListWidget(std::string fir, bool def);

private:
	tgui::VerticalLayout::Ptr m_verticalLayout;
	tgui::GrowHorizontalLayout::Ptr m_row1;
	tgui::GrowHorizontalLayout::Ptr m_row2;
	tgui::GrowHorizontalLayout::Ptr m_row3;
	tgui::GrowHorizontalLayout::Ptr m_row4;
	tgui::GrowHorizontalLayout::Ptr m_row5;

	tgui::Label::Ptr m_statusText;

	tgui::EditBox::Ptr m_tokenEntry;

	tgui::ComboBox::Ptr m_firSelector;
	tgui::EditBox::Ptr m_airportList;
	tgui::Button::Ptr m_resetButton;

	tgui::Button::Ptr m_rwyLocationButton;
	tgui::Button::Ptr m_rwyAssignButton;

	tgui::Button::Ptr m_settingsButton;

	tgui::FileDialog::Ptr m_fileDialog;

#ifdef _WIN32
	POINT clickOffset_{};
#endif // _WIN32
};

class GuiSettingWindow : public GuiWindow {
public:
	GuiSettingWindow(unsigned int width, unsigned int height, const std::string& title, Aras* aras);
	bool createWindow() override;
	void createSettingsWindowWidgets();

private:
	tgui::VerticalLayout::Ptr m_verticalLayout;
	POINT clickOffset_{};
};