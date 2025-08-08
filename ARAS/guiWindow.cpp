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

void GuiWindow::createBaseWindowLayout(const std::string& title)
{
	//GUI elements
	// Background
	sf::RectangleShape background;
	background.setSize(sf::Vector2f(m_width, m_height));
	background.setFillColor(sf::Color(40, 40, 40));
	m_drawables.push_back(std::move(std::make_unique<sf::RectangleShape>(background)));

	// Drag Area
	sf::RectangleShape dragArea;
	float dragAreaHeight = 30;
	dragArea.setSize(sf::Vector2f(m_width, dragAreaHeight));
	dragArea.setFillColor(sf::Color(50, 50, 50));
	dragArea.setPosition({ 0, 0 });
	m_drawables.push_back(std::move(std::make_unique<sf::RectangleShape>(dragArea)));

	// Icon
	iconTexture.resize({ m_icon.getSize().x, m_icon.getSize().y });
	iconTexture.update(m_icon);
	iconTexture.setSmooth(true);
	sf::Sprite iconSprite(iconTexture);
	float scale = dragAreaHeight * 0.7 / static_cast<float>(m_icon.getSize().x);
	iconSprite.setScale({ scale, scale });
	iconSprite.setOrigin(iconSprite.getLocalBounds().getCenter());
	iconSprite.setPosition({ dragAreaHeight / 2, dragAreaHeight / 2 });
	m_drawables.push_back(std::move(std::make_unique<sf::Sprite>(iconSprite)));

	// Title Text
	sf::Text titleText(m_font, title, 25);
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

	// Loading dependencies
	if (m_icon.loadFromFile("ressources/images/icon.png")) {
		//LOG
	}
	m_window.setIcon(m_icon);

	if (!m_font.openFromFile("ressources/fonts/arial.ttf")) {
		//LOG	
	}

	createBaseWindowLayout("Automatic Runway Assignement System");
	createMainWindowWidgets();


	return true;
}

void GuiMainWindow::createMainWindowWidgets()
{
	//TODO: (placeholder)
	sf::RectangleShape container;
	container.setSize(sf::Vector2f(m_width*0.9, m_height*0.8));
	container.setOrigin(container.getLocalBounds().getCenter());
	container.setPosition({ static_cast<float>(m_width) / 2, static_cast<float>(m_height) / 2 + 15});
	container.setFillColor(sf::Color(60, 60, 60));
	m_drawables.push_back(std::move(std::make_unique<sf::RectangleShape>(container)));

	m_grid = tgui::Grid::create();
	m_grid->setSize({ m_width * 0.9f, m_height * 0.8f });
	m_grid->setPosition({ m_width * 0.05f, m_height * 0.15f });
	
	tgui::Button::Ptr button1 = createButton("Button 1", { 0,0 }, { 200,100 }, {});
	m_grid->add(button1);
	m_grid->setWidgetCell(button1, 0, 0);
	m_grid->setWidgetAlignment(button1, tgui::Grid::Alignment::Center);

	tgui::Button::Ptr button2 = createButton("Button 2", {0,0}, {200,100}, {});
	m_grid->add(button2);
	m_grid->setWidgetCell(button2, 0, 1);
	m_grid->setWidgetAlignment(button2, tgui::Grid::Alignment::Center);

	tgui::Button::Ptr button3 = createButton("Button 3", { 0,0 }, { 200,100 }, {});
	m_grid->add(button3);
	m_grid->setWidgetCell(button3, 1, 0);
	m_grid->setWidgetAlignment(button3, tgui::Grid::Alignment::Center);

	m_gui.add(m_grid);

}
