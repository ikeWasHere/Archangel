#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
    const int wWidth = 1280;
    const int wHeight = 720;
    sf::RenderWindow window(sf::VideoMode({wWidth, wHeight}), "My Window");
    window.setFramerateLimit(60);

    // Initialize ImGui-SFML and create a clock for its internal timing
    if (!ImGui::SFML::Init(window))
    {
        std::cout << "Could not initialize window\n";
        return -1;
    }

    sf::Clock deltaClock;

    std::vector<sf::Shape> shapes;

    // Circle properties
    float circleRadius = 100.0f;
    int circleSegments = 32;
    float circleSpeedX = 1.0f;
    float circleSpeedY = 0.5f;
    bool drawCircle = true;
    bool drawText = true;
    float c[3] = {0.0f, 1.0f, 1.0f};

    sf::CircleShape circle(circleRadius, circleSegments);
    circle.setPosition({300.0f, 200.0f});

    // Load and display text
    sf::Font myFont;

    if (!myFont.openFromFile("fonts/Arial.ttf"))
    {
        std::cout << "Could not load font!\n";
        return -1;
    }

    // Set up text object
    sf::Text text(myFont, "This is my text", 24);
    text.setPosition({0, wHeight - 45.0f});

    // Main loop
    while (window.isOpen())
    {
        // Handle events
        while (auto event = window.pollEvent())
        {
            // Pass event to IMGui to be parsed
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }

        // Update ImGui
        ImGui::SFML::Update(window, deltaClock.restart());

        // ImGui windows
        ImGui::Begin("Shape Properties");

        ImGui::Text("Adjust circle properties:");
        ImGui::Separator();

        if (ImGui::SliderFloat("Radius", &circleRadius, 10.0f, 200.0f))
        {
            circle.setRadius(circleRadius);
        }

        if (ImGui::ColorEdit3("Color", c))
        {
            circle.setFillColor(sf::Color(
                static_cast<std::uint8_t>(c[0] * 255),
                static_cast<std::uint8_t>(c[1] * 255),
                static_cast<std::uint8_t>(c[2] * 255)));
        }

        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();

        // Set circle properties, because they may have been updated with the ui
        circle.setPointCount(circleSegments);
        circle.setRadius(circleRadius);

        // Convert from the ui floates to sfml Uint8
        circle.setFillColor(sf::Color(
            uint8_t(c[0] * 255),
            uint8_t(c[1] * 255),
            uint8_t(c[2] * 255)));

        circle.setPosition({circle.getPosition().x + circleSpeedX, circle.getPosition().y + circleSpeedY});

        if (circle.getPosition().x > wWidth)
        {
            circle.setPosition({300.0f, 200.0f});
        }

        // Render
        window.clear();
        if (drawCircle)
        {
            window.draw(circle);
        }
        if (drawText)
        {
            window.draw(text);
        }
        ImGui::SFML::Render(window); // Draw ui last so its on top
        window.display();
    }

    // Cleanup
    ImGui::SFML::Shutdown();
    return 0;
}
