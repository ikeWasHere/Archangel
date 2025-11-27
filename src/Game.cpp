#include "Game.h"

#include <iostream>

Game::Game(const std::string &config)
    : m_text(m_font, "Defualt", 18)
{
    init(config);
}

void Game::init(const std::string &path)
{
    // TODO read in confgi file hre
    // use premade PlayerCOnfig, Enemfyconfig and bulletconfig variables

    // set up default window parameters
    m_window.create(sf::VideoMode({1280, 720}), "Assignment 2");
    m_window.setKeyRepeatEnabled(false);
    m_window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(m_window))
    {
    }

    spawnPlayer();
}

std::shared_ptr<Entity> Game::player()
{
    return m_entities.getEntities("player").back();
}

void Game::run()
{
    // TODO: add pause functionality in here

    while (true)
    {
        m_entities.update();

        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        sUserInput();
        sEnemySpawner();
        sMovement();
        sCollision();
        sGUI();
        sRender();

        // increment the current frame
        m_currentFrame++;
    }
}

void Game::spawnPlayer()
{
    // TODO: Fill this data with config file

    auto e = m_entities.addEntity("player");
    e->add<CTransform>(Vec2f(200.0f, 200.0f), Vec2f(1.0f, 1.0f), 0.0f);
    e->add<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color(255, 0, 0), 4.0f);
    e->add<CInput>();
}

void Game::spawnEnemy()
{
    // TODO: Fill data with config file
    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    // TODO: spawn small enemies at the location of the input enemy e

    // when we create the smaller enemy, we have to read the values of the original enemy
    // - spawn a number of small enemies equal to the vertices of the original enemy
    // - set each small enemy to the same color as the original, half the size
    // - small enemies are worth double points of the original enemy
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f &target)
{
    // TODO: implement the spawning of a bullet which travels toward target
    //       -- bullet speed is given as a scalar speed
    //       -- you must set the velocity by using formula in notes
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // TODO: implement special weapon
}

void Game::sMovement()
{
    // TODO: implement all entity movement in this function
    //       you should read the m_player->cInput component to determine if the player is moving

    // Sample movement speed update for the player
    auto &transform = player()->get<CTransform>();
    transform.pos.x += transform.velocity.x;
    transform.pos.y += transform.velocity.y;
}

void Game::sLifespan()
{
    // TODO: implement all lifespan functionality
    //
    // for all entities
    //     if entity has no lifespan component, skip it
    //     if entity has > 0 remaining lifespan, subtract 1
    //     if it has lifespan and is alive
    //         scale its alpha channel properly
    //     if it has lifespan and its time is up
    //         destroy the entity
}

void Game::sCollision()
{
    // TODO: implement all proper collisions between entities
    //       be sure to use the collision radius, NOT the shape radius

    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("enemy"))
        {
            // do collision logic
        }

        for (auto e : m_entities.getEntities("smallEnemy"))
        {
            // do collision logic
        }
    }
}

void Game::sEnemySpawner()
{
    // TODO: code which implements enemy spawning should go here
}

void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");

    ImGui::Text("blah blah");

    ImGui::End();
}

void Game::sRender()
{
    if (!m_window.isOpen())
    {
        return;
    }

    m_window.clear();

    // draw player

    // draw ui last
    ImGui::SFML::Render(m_window);

    m_window.display();
}

void Game::sUserInput()
{
    // TODO: handle user input here
    //       note that you should only be setting the player's input component
    //       you should not implement the player's movement logic here
    //       the movement system will read the variables you set in this function

    while (auto event = m_window.pollEvent())
    {
        // pass the event to imgui to be parsed
        ImGui::SFML::ProcessEvent(m_window, *event);

        // this event triggers when the window is closed
        if (event->is<sf::Event::Closed>())
        {
            std::exit(0);
        }

        // this event is triggered when a key is pressed
        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            // print the key that was pressed to the console
            std::cout << "Key pressed = " << int(keyPressed->scancode) << "\n";

            if (keyPressed->scancode == sf::Keyboard::Scancode::W)
            {
                // TODO: set player's input component UP to true
                std::cout << "W Key Pressed\n";
            }
        }

        if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            Vec2f mpos(mousePressed->position);

            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                // TODO: call spawnBullet here
                std::cout << "Left mouse (" << mpos.x << ", " << mpos.y << ")\n";
            }
            else if (mousePressed->button == sf::Mouse::Button::Right)
            {
                // TODO: call special weapon here
                std::cout << "Right mouse (" << mpos.x << ", " << mpos.y << ")\n";
            }
        }
    }
}
