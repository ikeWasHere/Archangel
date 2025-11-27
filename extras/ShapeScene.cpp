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

    if (!myFont.openFromFile("res/fonts/Arial.ttf"))
    {
        std::cout << "Could not load font!\n";
        return -1;
    }

    while (window.isOpen())
    {

        while (auto event = window.pollEvent())
        {
            // Pass event to IMGui to be parsed
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();
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
            sf::Vector2f const scale = current.shape->getScale();
            float uniformScale = scale.x;
            if (ImGui::SliderFloat("Scale", &uniformScale, 0.1f, 5.0f))
            {
                current.shape->setScale(sf::Vector2f{uniformScale, uniformScale});
            }
            ImGui::DragFloat2("Velocity", &current.velocity.x, 0.1f);

            sf::Color const col = current.shape->getFillColor();
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
            auto bounds = info.shape->getGlobalBounds();

            if (bounds.position.x <= 0.f ||
                bounds.position.x + bounds.size.x >= static_cast<float>(wWidth))
            {
                info.velocity.x *= -1.f;
            }

            if (bounds.position.y <= 0.f ||
                bounds.position.y + bounds.size.y >= static_cast<float>(wHeight))
            {
                info.velocity.y *= -1.f;
            }
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

        for (auto &info : shapeData)
        {
            if (!info.visible)
                continue;
            auto bounds = info.shape->getGlobalBounds();
            sf::Vector2f center{
                bounds.position.x + bounds.size.x * 0.5f,
                bounds.position.y + bounds.size.y * 0.5f};

            sf::Text nameLabel(myFont, info.name, 18);
            sf::FloatRect textBounds = nameLabel.getLocalBounds();
            nameLabel.setOrigin({textBounds.position.x + textBounds.size.x * 0.5f,
                                 textBounds.position.y + textBounds.size.y * 0.5f

            });
            nameLabel.setPosition(center);
            window.draw(nameLabel);
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
    auto circle_shape1 = std::make_unique<sf::CircleShape>(100.f, 32);
    circle_shape1->setPosition({300.f, 200.f});
    circle_shape1->setFillColor(sf::Color::Blue);

    ShapeData circle1;
    circle1.name = "CBlue";
    circle1.shape = circle_shape1.get();
    circle1.velocity = {1.f, 0.5f};
    circle1.visible = true;

    auto circle_shape2 = std::make_unique<sf::CircleShape>(120.f, 50);
    circle_shape2->setPosition({370.f, 150.f});
    circle_shape2->setFillColor(sf::Color::Green);

    ShapeData circle2;
    circle2.name = "CGreen";
    circle2.shape = circle_shape2.get();
    circle2.velocity = {2.f, 1.f};
    circle2.visible = true;

    shapes.push_back(std::move(circle_shape1));
    shapes.push_back(std::move(circle_shape2));
    shapeData.push_back(circle1);
    shapeData.push_back(circle2);

    auto rect_shape1 = std::make_unique<sf::RectangleShape>(sf::Vector2f{80.0f, 150.0f});
    rect_shape1->setPosition({100.f, 100.f});
    rect_shape1->setFillColor(sf::Color::Yellow);

    ShapeData rect1;
    rect1.name = "RYellow";
    rect1.shape = rect_shape1.get();
    rect1.velocity = {1.2f, 1.f};
    rect1.visible = true;

    auto rect_shape2 = std::make_unique<sf::RectangleShape>(sf::Vector2f{200.0f, 100.0f});
    rect_shape2->setPosition({300.f, 400.f});
    rect_shape2->setFillColor(sf::Color::Red);

    ShapeData rect2;
    rect2.name = "RRed";
    rect2.shape = rect_shape2.get();
    rect2.velocity = {1.9f, 2.f};
    rect2.visible = true;

    shapes.push_back(std::move(rect_shape1));
    shapes.push_back(std::move(rect_shape2));
    shapeData.push_back(rect1);
    shapeData.push_back(rect2);
}
