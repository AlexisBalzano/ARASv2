#include "guiWindow.h"


#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#endif

GuiWindow::GuiWindow(unsigned int width, unsigned int height, const std::string& title)
	: m_width(width), m_height(height), m_title(title) {}

GuiWindow::~GuiWindow()
{
	if (m_window.isOpen())
		m_window.close();
}

bool GuiWindow::createWindow()
{
	m_window.create(sf::VideoMode({ m_width, m_height }), m_title, sf::Style::Default);
	if (!m_window.isOpen())
		return false;
	m_window.setFramerateLimit(60);
	m_gui.setTarget(m_window);
	m_hwnd = m_window.getNativeHandle();

	if (!m_font.openFromFile("fonts/arial.ttf")) {
		//LOG
	}
	

	return true;
}

std::optional<sf::Event> GuiWindow::pollWindowEvent()
{
	return m_window.pollEvent();
}

void GuiWindow::processEvents(const sf::Event& event)
{
	if (event.is<sf::Event::Closed>()) {
		m_window.close();
	}

	// Dragging the window
	const sf::Event::MouseButtonPressed* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
	if (mouseEvent != nullptr && mouseEvent->button == sf::Mouse::Button::Left) {
		if (mouseEvent->position.y < 30) {
			m_dragging = true;
			POINT mousePos;
			GetCursorPos(&mousePos);
			RECT winRect;
			GetWindowRect(m_hwnd, &winRect);
			m_clickOffset.x = mousePos.x - winRect.left;
			m_clickOffset.y = mousePos.y - winRect.top;
		}
	}
	if (event.is<sf::Event::MouseButtonReleased>()) {
		auto mouseEvent = event.getIf<sf::Event::MouseButtonReleased>();
		if (mouseEvent->button == sf::Mouse::Button::Left) {
			m_dragging = false;
		}
	}

	m_gui.handleEvent(event);
}

void GuiWindow::render()
{
	m_window.clear();

	for (const auto& drawable : m_drawables) {
		m_window.draw(*drawable);
	}

	updateDrag();
	m_gui.draw();
	m_window.display();
}

bool GuiWindow::isOpen() const
{
	if (m_window.isOpen())
		return true;
	return false;
}

#ifdef _WIN32
void GuiWindow::updateDrag()
{
	if (m_dragging) {
		POINT mousePos;
		GetCursorPos(&mousePos);
		int xValue = mousePos.x - m_clickOffset.x;
		int yValue = mousePos.y - m_clickOffset.y;
		SetWindowPos(m_hwnd, nullptr, xValue, yValue, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
#endif

tgui::Button::Ptr GuiWindow::createButton(const std::string& buttonText, tgui::Vector2f position, tgui::Vector2f size, ButtonColors colors)
{
	tgui::Button::Ptr button = tgui::Button::create(buttonText);
	button->setSize(size);
	button->setPosition(position);
	button->getRenderer()->setTextColor(colors.text);
	button->getRenderer()->setTextColorHover(colors.textHover);
	button->getRenderer()->setTextColorDown(colors.textDown);
	button->getRenderer()->setBackgroundColor(colors.background);
	button->getRenderer()->setBackgroundColorHover(colors.backgroundHover);
	button->getRenderer()->setBackgroundColorDown(colors.backgroundDown);
	button->getRenderer()->setBorders({ 0, 0, 0, 0 });
	return button;
}


GuiMainWindow::GuiMainWindow(unsigned int width, unsigned int height, const std::string& title)
	: GuiWindow(width, height, title)
{
	m_dragging = false;
	m_hwnd = nullptr;
}

bool GuiMainWindow::createWindow()
{
	m_window.create(sf::VideoMode({ m_width, m_height }), m_title, sf::Style::None);
	if (!m_window.isOpen())
		return false;
	m_window.setFramerateLimit(60);
	m_gui.setTarget(m_window);
	m_hwnd = m_window.getNativeHandle();

	if (m_icon.loadFromFile("ressources/images/icon.png")) {
		//LOG
	}
	m_window.setIcon(m_icon);

	if (!m_font.openFromFile("ressources/fonts/arial.ttf")) {
		//LOG	
	}

	//GUI elements
	// Background
	sf::RectangleShape background;
	background.setSize(sf::Vector2f(m_width, m_height));
	background.setFillColor(sf::Color(40,40,40));
	m_drawables.push_back(std::move(std::make_unique<sf::RectangleShape>(background)));

	// Drag Area
	sf::RectangleShape dragArea;
	float dragAreaHeight = 30;
	dragArea.setSize(sf::Vector2f(m_width, dragAreaHeight));
	dragArea.setFillColor(sf::Color(50, 50, 50));
	dragArea.setPosition({0, 0});
	m_drawables.push_back(std::move(std::make_unique<sf::RectangleShape>(dragArea)));

	// Icon
	iconTexture.resize({ m_icon.getSize().x, m_icon.getSize().y });
	iconTexture.update(m_icon);
	iconTexture.setSmooth(true);
	sf::Sprite iconSprite(iconTexture);
	float scale = dragAreaHeight*0.7 / static_cast<float>(m_icon.getSize().x);
	iconSprite.setScale({scale, scale});
	iconSprite.setOrigin(iconSprite.getLocalBounds().getCenter());
	iconSprite.setPosition({ dragAreaHeight / 2, dragAreaHeight / 2 });
	m_drawables.push_back(std::move(std::make_unique<sf::Sprite>(iconSprite)));

	// Title Text
	sf::Text titleText(m_font, "Automatic Runway Assignement System", 25);
	titleText.setFillColor(sf::Color::White);
	titleText.setOrigin(titleText.getLocalBounds().getCenter());
	titleText.setPosition({ static_cast<float>(m_width / 2), static_cast<float>(dragAreaHeight / 2) });
	m_drawables.push_back(std::move(std::make_unique<sf::Text>(titleText)));

	// Close Button
	ButtonColors closeButtonColors;
	closeButtonColors.backgroundHover = tgui::Color::Red;
	m_closeButton = createButton("X", { m_width - dragAreaHeight , 0 }, { dragAreaHeight, dragAreaHeight }, closeButtonColors);
	m_closeButton->onClick([this] {
		m_window.close();
	});
	m_gui.add(m_closeButton);

	// Minimise Button
	m_minimiseButton = createButton("-", { m_width - 2 * dragAreaHeight, 0 }, { dragAreaHeight, dragAreaHeight }, {});
	m_minimiseButton->onClick([this] {
#if defined(_WIN32)
		// Windows: Use WinAPI to minimize the window
		ShowWindow(m_hwnd, SW_MINIMIZE);
#elif defined(__linux__)
		// Linux (X11): Use X11 to minimize the window
		::Display* display = XOpenDisplay(nullptr);
		if (display) {
			::Window win = m_window.getSystemHandle();
			XIconifyWindow(display, win, DefaultScreen(display));
			XFlush(display);
			XCloseDisplay(display);
		}
#endif
	});
	m_gui.add(m_minimiseButton);

	m_grid = tgui::Grid::create();

	return true;
}
