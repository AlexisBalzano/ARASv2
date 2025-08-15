#include "GuiWindow.h"
#include "Aras.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#endif

GuiWindow::GuiWindow(unsigned int width, unsigned int height, const std::string& title, Aras* aras)
	: m_width(width), m_height(height), m_title(title), m_aras(aras)
{
#ifdef _WIN32
	m_dragging = false;
	m_hwnd = nullptr;
#endif
}

GuiWindow::~GuiWindow()
{
	if (m_window.isOpen())
		m_window.close();
}

bool GuiWindow::createWindow()
{
	sf::ContextSettings settings;
	settings.antiAliasingLevel = 8;
	m_window.create(sf::VideoMode({ m_width, m_height }), m_title, sf::Style::Default, sf::State::Windowed, settings);
	if (!m_window.isOpen())
		return false;
	m_window.setFramerateLimit(60);

	m_gui.setTarget(m_window);
	m_hwnd = m_window.getNativeHandle();

#ifdef _WIN32
	// Set the window to be transparent
	SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_hwnd, RGB(sf::Color::Transparent.r, sf::Color::Transparent.g, sf::Color::Transparent.b), 0, LWA_COLORKEY);
#endif

	// Loading dependencies
	if (m_icon.loadFromFile("ressources/images/icon.png")) {
		//LOG
	}
	m_window.setIcon(m_icon);

	if (!m_font.openFromFile("ressources/fonts/arial.ttf")) {
		//LOG	
	}

	m_gui.setFont("ressources/fonts/arial.ttf");

	return true;
}

void GuiWindow::createBaseWindowLayout(const std::string& title)
{
	//GUI elements
	// Background
	backgroundCanvas = tgui::CanvasSFML::create({ m_width, m_height });
	m_gui.add(backgroundCanvas);
	backgroundCanvas->clear(sf::Color::Transparent);

	RoundedRectangle rect({ static_cast<float>(m_width), static_cast<float>(m_height) }, 10);
	rect.setFillColor(Colors::LightGrey);
	rect.setOrigin(rect.getLocalBounds().getCenter());
	rect.setPosition({ static_cast<float>(m_width) / 2, static_cast<float>(m_height) / 2});
	backgroundCanvas->draw(rect);

	backgroundCanvas->display();


	// Drag Area
	float dragAreaHeight = 30;

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
	m_window.clear(sf::Color::Transparent);

	m_gui.draw();
	for (const auto& drawable : m_drawables) {
		m_window.draw(*drawable);
	}

	updateDrag();
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
	button->getRenderer()->setRoundedBorderRadius(10);
	button->setMouseCursor(tgui::Cursor::Type::Hand);
	return button;
}


GuiMainWindow::GuiMainWindow(unsigned int width, unsigned int height, const std::string& title, Aras* aras)
	: GuiWindow(width, height, title, aras) {}

bool GuiMainWindow::createWindow()
{
	sf::ContextSettings settings;
	settings.antiAliasingLevel = 8;
	m_window.create(sf::VideoMode({ m_width, m_height }), m_title, sf::Style::None, sf::State::Windowed, settings);
	if (!m_window.isOpen())
		return false;
	m_window.setFramerateLimit(60);
	m_gui.setTarget(m_window);
	m_hwnd = m_window.getNativeHandle();

#ifdef _WIN32
	// Set the window to be transparent
	SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_hwnd, RGB(sf::Color::Transparent.r, sf::Color::Transparent.g, sf::Color::Transparent.b), 0, LWA_COLORKEY);
#endif

	// Loading dependencies
	if (m_icon.loadFromFile("ressources/images/icon.png")) {
		//LOG
	}
	m_window.setIcon(m_icon);

	if (!m_font.openFromFile("ressources/fonts/arial.ttf")) {
		//LOG	
	}

	m_gui.setFont("ressources/fonts/arial.ttf");

	createBaseWindowLayout("Automatic Runway Assignement System");
	createMainWindowWidgets();


	return true;
}

void GuiMainWindow::createMainWindowWidgets()
{
	// Dark Rectangle background
	RoundedRectangle rect({ static_cast<float>(m_width), static_cast<float>(m_height) }, 10);
	rect.setFillColor(Colors::Grey);
	rect.setSize({ 900, 60 });
	rect.setOrigin(rect.getLocalBounds().getCenter());
	rect.setPosition({ static_cast<float>(m_width) / 2, 102 });
	backgroundCanvas->draw(rect);
	rect.setPosition({ static_cast<float>(m_width) / 2, 192 });
	backgroundCanvas->draw(rect);
	rect.setPosition({ static_cast<float>(m_width) / 2, 282 });
	backgroundCanvas->draw(rect);

	// Vertical Layout
	m_verticalLayout = tgui::VerticalLayout::create();
	m_verticalLayout->setSize({ m_width * 0.9f, m_height * 0.8f });
	m_verticalLayout->setPosition({ m_width * 0.05f, m_height * 0.1f });
	m_verticalLayout->addSpace(1);

	// Row 1
	m_row1 = tgui::GrowHorizontalLayout::create();
	m_row1->setSize({ m_width * 0.9f, m_height * 0.16f });
	m_row1->getRenderer()->setPadding({ 20, 0, 20, 0 });
	
	// Status Text
	m_statusText = tgui::Label::create("Status: Ready");
	m_statusText->setTextSize(20);
	m_statusText->getRenderer()->setTextColor(tgui::Color::Green);
	m_row1->add(m_statusText);
	m_verticalLayout->add(m_row1);
	m_verticalLayout->addSpace(1);
	

	// Row 2
	m_row2 = tgui::GrowHorizontalLayout::create();
	m_row2->setSize({ m_width * 0.9f, m_height * 0.16f });
	m_row2->getRenderer()->setPadding({ 20, 0, 20, 0 });

	// Token Label
	tgui::Label::Ptr tokenLabel = tgui::Label::create("API Token: ");
	tokenLabel->setTextSize(20);
	tokenLabel->getRenderer()->setTextColor(tgui::Color::White);
	tokenLabel->getRenderer()->setPadding({ 0, 8, 25, 0 });
	m_row2->add(tokenLabel);

	// Token Entry
	m_tokenEntry = tgui::EditBox::create();
	m_tokenEntry->setSize({ 710, 30 });
	std::string token = m_aras->getTokenConfig();
	if (!token.empty()) {
		m_tokenEntry->setText(token);
	}
	else {
		m_tokenEntry->setDefaultText("Enter API token");
	}
	m_tokenEntry->setPasswordCharacter('*');
	m_tokenEntry->setTextSize(20);
	m_tokenEntry->getRenderer()->setRoundedBorderRadius(10);
	m_tokenEntry->setMouseCursor(tgui::Cursor::Type::Text);
	m_tokenEntry->onReturnOrUnfocus([this] {
		m_aras->saveToken(m_tokenEntry->getText().toStdString());
		});
	m_row2->add(m_tokenEntry);
	m_verticalLayout->add(m_row2);
	m_verticalLayout->addSpace(1.2);


	// Row 3
	m_row3 = tgui::GrowHorizontalLayout::create();
	m_row3->setSize({ m_width * 0.9f, m_height * 0.016f });
	m_row3->getRenderer()->setPadding({ 20, 0, 20, 0 });


	// FIR Airport Label
	tgui::Label::Ptr firAirportLabel = tgui::Label::create("FIR Airports: ");
	firAirportLabel->setTextSize(20);
	firAirportLabel->getRenderer()->setTextColor(tgui::Color::White);
	firAirportLabel->getRenderer()->setPadding({ 0, 8, 0, 0 });
	m_row3->add(firAirportLabel);

	// FIR Selector
	m_firSelector = tgui::ComboBox::create();
	m_firSelector->setSize({ 100, 30 });
	m_firSelector->setTextSize(20);
	std::vector<std::string> firs = m_aras->getFIRs();
	if (firs.empty()) {
		m_firSelector->addItem("No FIRs available");
	}
	else {
		for (const auto& fir : firs) {
			m_firSelector->addItem(fir);
		}
		m_firSelector->setSelectedItemByIndex(0);
	}
	m_firSelector->getRenderer()->setRoundedBorderRadius(10);
	m_firSelector->setMouseCursor(tgui::Cursor::Type::Hand);
	m_firSelector->onItemSelect([this] {
		std::string selectedFIR = m_firSelector->getSelectedItem().toStdString();
		updateAirportListWidget(selectedFIR);
		});
	m_row3->add(m_firSelector);

	// Airport List
	m_airportList = tgui::EditBox::create();
	m_airportList->setSize({ 500, 30 });
	if (firs.empty()) {
		m_airportList->setDefaultText("No FIRs available");
	}
	else {
		updateAirportListWidget(firs[0]);
	}
	m_airportList->setTextSize(20);
	m_airportList->getRenderer()->setRoundedBorderRadius(10);
	m_airportList->setMouseCursor(tgui::Cursor::Type::Text);
	m_airportList->onReturnOrUnfocus([this] {
		m_aras->updateAirportsList(m_firSelector->getSelectedItem().toStdString(), m_airportList->getText().toStdString());
		});
	m_row3->add(m_airportList);

	// Reset Button
	ButtonColors resetButtonColors;
	resetButtonColors.background = Colors::Red;
	resetButtonColors.backgroundHover = tgui::Color::Red;
	resetButtonColors.text = tgui::Color::White;
	m_resetButton = createButton("Reset", { m_width * 0.05f, m_height * 0.85f }, { 70, 30 }, resetButtonColors);
	m_resetButton->onClick([this] {
		m_aras->resetAirportsList();
		});
	m_row3->add(m_resetButton);
	m_verticalLayout->add(m_row3);
	m_verticalLayout->addSpace(1.2);


	// Row 4
	m_row4 = tgui::GrowHorizontalLayout::create();
	m_row4->setSize({ m_width * 0.9f, m_height * 0.16f });
	m_row4->getRenderer()->setPadding({ 20, 0, 20, 0 });

	// Rwy location Button
	ButtonColors arasButtonColors;
	arasButtonColors.background = Colors::Blue;
	arasButtonColors.text = tgui::Color::White;
	arasButtonColors.backgroundHover = Colors::Yellow;
	arasButtonColors.textHover = tgui::Color::Black;
	m_rwyLocationButton = createButton("Choose .rwy location", { m_width * 0.2f, m_height * 0.85f }, { 150, 30 }, arasButtonColors);
	// OnClick inside GUI
	m_row4->add(m_rwyLocationButton);

	// Runway Assign Button
	m_rwyAssignButton = createButton("Assign runways", { m_width * 0.35f, m_height * 0.85f }, { 150, 30 }, arasButtonColors);
	m_rwyAssignButton->onClick([this] {
		m_aras->assignRunways();
	});
	m_row4->add(m_rwyAssignButton);
	m_verticalLayout->add(m_row4);
	m_verticalLayout->addSpace(0.5);


	// Row 5
	m_row5 = tgui::GrowHorizontalLayout::create();
	m_row5->setSize({ m_width * 0.9f, m_height * 0.16f });
	m_row5->getRenderer()->setPadding({ 20, 0, 20, 0 });

	// Settings Button
	m_settingsButton = createButton("Settings", { m_width * 0.5f, m_height * 0.85f }, { 150, 30 }, arasButtonColors);
	m_settingsButton->onClick([this] {
		m_aras->openSettings();
	});
	m_row5->add(m_settingsButton);
	m_verticalLayout->add(m_row5);


	m_gui.add(m_verticalLayout);
}

void GuiMainWindow::updateAirportListWidget(std::string fir)
{
	std::vector<std::string> airports = m_aras->getAirports(fir);
	std::string airportListText;
	for (const auto& airport : airports) {
		airportListText += airport + ", ";
	}
	if (!airportListText.empty()) {
		// Remove the last comma and space
		airportListText = airportListText.substr(0, airportListText.length() - 2);
	}
	m_airportList->setText(airportListText);

}

GuiSettingWindow::GuiSettingWindow(unsigned int width, unsigned int height, const std::string& title, Aras* aras)
	: GuiWindow(width, height, title, aras) {}

bool GuiSettingWindow::createWindow()
{
	sf::ContextSettings settings;
	settings.antiAliasingLevel = 8;
	m_window.create(sf::VideoMode({ m_width, m_height }), m_title, sf::Style::None, sf::State::Windowed, settings);
	if (!m_window.isOpen())
		return false;
	m_window.setFramerateLimit(60);
	m_gui.setTarget(m_window);
	m_hwnd = m_window.getNativeHandle();

#ifdef _WIN32
	// Set the window to be transparent
	SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_hwnd, RGB(sf::Color::Transparent.r, sf::Color::Transparent.g, sf::Color::Transparent.b), 0, LWA_COLORKEY);
#endif

	// Loading dependencies
	if (m_icon.loadFromFile("ressources/images/icon.png")) {
		//LOG
	}
	m_window.setIcon(m_icon);

	if (!m_font.openFromFile("ressources/fonts/arial.ttf")) {
		//LOG	
	}

	m_gui.setFont("ressources/fonts/arial.ttf");
	
	createBaseWindowLayout("Settings");
	createSettingsWindowWidgets();


	return true;
}

void GuiSettingWindow::createSettingsWindowWidgets()
{
	// Dark Rectangle background
	RoundedRectangle rect({ static_cast<float>(m_width), static_cast<float>(m_height) - 30}, 10);
	rect.setFillColor(Colors::Grey);
	rect.setOrigin(rect.getLocalBounds().getCenter());
	rect.setPosition({ static_cast<float>(m_width) / 2, static_cast<float>(m_height) / 2 + 15});
	backgroundCanvas->draw(rect);

	// Vertical Layout
	m_verticalLayout = tgui::VerticalLayout::create();
	m_verticalLayout->setSize({ m_width * 0.9f, m_height * 0.8f });
	m_verticalLayout->setPosition({ m_width * 0.05f, m_height * 0.1f });
	m_verticalLayout->addSpace(1);

	
	m_gui.add(m_verticalLayout);
}
