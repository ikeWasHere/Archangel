#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>

template <typename T>
class Vec2
{
public:
    T x = 0;
    T y = 0;

    Vec2() = default;

    Vec2(T xin, T yin)
        : x(xin), y(yin)
    {
    }

    void print() const
    {
        std::cout << x << " " << y << '\n';
    }

    // Constructor to convert from sf::Vector2
    template <typename U>
    Vec2(const sf::Vector2<U> &vec)
        : x(static_cast<T>(vec.x)), y(static_cast<T>(vec.y))
    {
    }

    // Allow automatic conversion to sf::Vector2
    // Lets you pass Vev2 into sfml functions
    operator sf::Vector2<T>() const
    {
        return sf::Vector2<T>(x, y);
    }

    Vec2 operator+(const Vec2 &rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2 &rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator/(const T val) const
    {
        return Vec2(x / val, y / val);
    }

    Vec2 operator*(const T val) const
    {
        return Vec2(x * val, y * val);
    }

    bool operator==(const Vec2 &rhs) const
    {
        return (x == rhs.x && y == rhs.y);
    }

    bool operator!=(const Vec2 &rhs) const
    {

        return !(*this == rhs);
    }

    void operator+=(const Vec2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }

    void operator-=(const Vec2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }

    void operator*=(const T val)
    {
        x *= val;
        y *= val;
    }

    void operator/=(const T val)
    {
        x /= val;
        y /= val;
    }

    float dist(const Vec2 &rhs) const
    {
        float dx = rhs.x - x;
        float dy = rhs.y - y;
        return std::sqrtf(dx * dx + dy * dy);
    }

    float length() const
    {
        return std::sqrtf(x * x + y * y);
    }

    void normalize()
    {
        float l = length();
        if (l == 0.0f)
            return;
        x /= l;
        y /= l;
    }
};
