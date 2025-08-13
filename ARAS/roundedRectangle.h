#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

class RoundedRectangle : public sf::Shape
{
public:
    explicit RoundedRectangle(sf::Vector2f size = { 0, 0 }, float cornerRadius = 10.f) : m_size(size), m_cornerRadius(cornerRadius)
    {
        update();
    }

    void setSize(sf::Vector2f size)
    {
        m_size = size;
        update();
    }

    sf::Vector2f getSize() const
    {
        return m_size;
    }

    std::size_t getPointCount() const override
    {
        return 40; // fixed, but could be an attribute of the class if needed
    }

    sf::Vector2f getPoint(std::size_t index) const override
    {
        static constexpr float pi = 3.141592654f;

        float actualWidth = m_size.x - 2 * m_cornerRadius;
        float actualHeight = m_size.y - 2 * m_cornerRadius;

        float x;
        float y;

        if (index < 10) {
            x = m_cornerRadius * std::sin(2 * pi * index / getPointCount()) + actualWidth;
            y = m_cornerRadius * std::cos(2 * pi * index / getPointCount());
        }
        else if (index < 20) {
            x = m_cornerRadius * std::sin(2 * pi * (index) / getPointCount()) + actualWidth;
            y = m_cornerRadius * std::cos(2 * pi * (index) / getPointCount()) - actualHeight;
        }
        else if (index < 30) {
            x = m_cornerRadius * std::sin(2 * pi * (index) / getPointCount());
            y = m_cornerRadius * std::cos(2 * pi * (index) / getPointCount()) - actualHeight;
        }
        else {
            x = m_cornerRadius * std::sin(2 * pi * (index) / getPointCount());
            y = m_cornerRadius * std::cos(2 * pi * (index) / getPointCount());
        }
        return sf::Vector2f(x, y);
    }

private:
    sf::Vector2f m_size;
	float m_cornerRadius;
};