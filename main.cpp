#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>
#include <vector>
#include <memory>
#include <string>

struct ShapeData
{
    std::string name;
    sf::Shape *shape = nullptr;
    sf::Vector2f velocity{0.f, 0.f};
    bool visible = true;
};

void initShapes(std::vector<std::unique_ptr<sf::Drawable>> &shapes,
                std::vector<ShapeData> &shapeData);

int main()
{
    const int wWidth = 800;
    const int wHeight = 700;
    sf::RenderWindow window(sf::VideoMode({wWidth, wHeight}), "My Window");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window))
    {
        std::cout << "Could not initialize window\n";
        return -1;
    }

    sf::Clock deltaClock;

    std::vector<std::unique_ptr<sf::Drawable>> shapes;
    std::vector<ShapeData> shapeData;

    initShapes(shapes, shapeData);

    sf::Font myFont;

    if (!myFont.openFromFile("fonts/Arial.ttf"))
    {
        std::cout << "Could not load font!\n";
        return -1;
    }

    sf::Text text(myFont, "This is my text", 24);
    text.setPosition({0, wHeight - 45.0f});

    while (window.isOpen())
    {

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

        ImGui::SFML::Update(window, deltaClock.restart());

        static int selectedIndex = 0;
        if (shapeData.empty())
            selectedIndex = -1;

        ImGui::Begin("Shape Properties");

        if (selectedIndex >= 0 && selectedIndex < (int)shapeData.size())
        {
            ShapeData &current = shapeData[selectedIndex];

            if (ImGui::BeginCombo("Shape", current.name.c_str()))
            {
                for (int i = 0; i < (int)shapeData.size(); i++)
                {
                    bool isSelected = (i == selectedIndex);
                    if (ImGui::Selectable(shapeData[i].name.c_str(), isSelected))
                        selectedIndex = i;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("Draw Shape", &current.visible);
            sf::Vector2f scale = current.shape->getScale();
            float uniformScale = scale.x;
            if (ImGui::SliderFloat("Scale", &uniformScale, 0.1f, 5.0f))
            {
                current.shape->setScale(sf::Vector2f{uniformScale, uniformScale});
            }
            ImGui::DragFloat2("Velocity", &current.velocity.x, 0.1f);

            sf::Color col = current.shape->getFillColor();
            float color[3] = {col.r / 255.f, col.g / 255.f, col.b / 255.f};
            if (ImGui::ColorEdit3("Color", color))
            {
                current.shape->setFillColor(sf::Color(
                    static_cast<std::uint8_t>(color[0] * 255),
                    static_cast<std::uint8_t>(color[1] * 255),
                    static_cast<std::uint8_t>(color[2] * 255)));
            }

            char nameBuff[32];
            std::snprintf(nameBuff, sizeof(nameBuff), "%s", current.name.c_str());
            if (ImGui::InputText("Name", nameBuff, sizeof(nameBuff)))
            {
                current.name = nameBuff;
            }
        }

        ImGui::Separator();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();

        for (auto &info : shapeData)
        {
            if (!info.visible)
                continue;
            info.shape->move(info.velocity);
        }

        // Render order matters
        window.clear();
        for (std::size_t i = 0; i < shapes.size(); i++)
        {
            if (!shapeData[i].visible)
                continue;
            window.draw(*shapes[i]);
        }
        window.draw(text);

        for (auto &info : shapeData)
        {
            if (!info.visible)
                continue;

            // TODO: Implement name label follow on shape
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}

void initShapes(std::vector<std::unique_ptr<sf::Drawable>> &shapes,
                std::vector<ShapeData> &shapeData)
{
    auto circle = std::make_unique<sf::CircleShape>(100.f, 32);
    circle->setPosition({300.f, 200.f});
    circle->setFillColor(sf::Color::Blue);

    ShapeData circle1;
    circle1.name = "CBlue";
    circle1.shape = circle.get();
    circle1.velocity = {1.f, 0.5f};
    circle1.visible = true;

    shapes.push_back(std::move(circle));
    shapeData.push_back(circle1);

    auto rect = std::make_unique<sf::RectangleShape>(sf::Vector2f{150.0f, 80.0f});
    rect->setPosition({100.f, 100.f});
    rect->setFillColor(sf::Color::Red);

    ShapeData rect1;
    rect1.name = "RRed";
    rect1.shape = rect.get();
    rect1.velocity = {1.f, 0.5f};
    rect1.visible = true;

    shapes.push_back(std::move(rect));
    shapeData.push_back(rect1);
}
