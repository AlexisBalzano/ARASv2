#include "guiWindow.h"

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

	if (!m_font.openFromFile("fonts/arial.ttf")) {
		//LOG
		//return false;
	}

	return true;
}

std::optional<sf::Event> GuiWindow::pollWindowEvent()
{
	return m_window.pollEvent();
}

void GuiWindow::processEvents(const sf::Event& event)
{
	if (event.is<sf::Event::Closed>())
		m_window.close();

	m_gui.handleEvent(event);
}

void GuiWindow::render()
{
	m_window.clear();
	m_gui.draw();
	m_window.display();
}

bool GuiWindow::isOpen() const
{
	if (m_window.isOpen())
		return true;
	return false;
}

//sf::RenderWindow window(sf::VideoMode({ 1000, 500 }), "ARAS", sf::Style::Default);
//tgui::Gui gui(window);
//
//auto verticalContainer = tgui::VerticalLayout::create({ "100%,100%" });
//std::vector<tgui::Button::Ptr> buttons;
//
//for (size_t i = 0; i < 3; i++)
//{
//    auto horizontalContainer = tgui::HorizontalLayout::create({ "100%,30%" });
//    for (size_t j = 0; j < 3; j++)
//    {
//        tgui::Button::Ptr button = tgui::Button::create(std::to_string(3 * i + j));
//        button->onClick([i, j]() {
//            std::cout << "Button " << 3 * i + j << " clicked!" << std::endl;
//            });
//        buttons.push_back(button);
//        horizontalContainer->add(buttons[buttons.size() - 1]);
//    }
//    verticalContainer->add(horizontalContainer);
//}
//gui.add(verticalContainer);
//
//while (window.isOpen())
//{
//    while (const std::optional event = window.pollEvent())
//    {
//        if (event->is<sf::Event::Closed>())
//            window.close();
//
//        gui.handleEvent(*event);
//    }
//
//    window.clear();
//    gui.draw();
//    window.display();
//}