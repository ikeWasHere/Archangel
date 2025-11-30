#include "Game.h"

#include <fstream>
#include <iostream>
#include <random>
#include <string>

Game::Game(const std::string &config) : m_text(m_font, "Defualt", 18)
{
    init(config);
}

void Game::init(const std::string &path)
{
    std::string type;
    int wWidth = 1280;
    int wHeight = 720;
    int fps = 60;

    std::string fontPath;
    int fontSize;

    std::ifstream inputFile(path);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Could not open the file!\n";
        return;
    }

    // Reading in data from config file
    if (inputFile >> type && type == "Window")
    {
        inputFile >> wWidth >> wHeight >> fps;
    }
    if (inputFile >> type && type == "Font")
    {
        inputFile >> fontPath >> fontSize;
    }
    if (inputFile >> type && type == "Player")
    {
        inputFile >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.FR >>
            m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >>
            m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V >> m_playerConfig.S;
    }
    if (inputFile >> type && type == "Enemy")
    {
        inputFile >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.OR >> m_enemyConfig.OG >>
            m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >>
            m_enemyConfig.L >> m_enemyConfig.SI >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX;
    }
    if (inputFile >> type && type == "Bullet")
    {
        inputFile >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.FR >>
            m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >>
            m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L >>
            m_bulletConfig.S;
    }

    m_window.create(
        sf::VideoMode({static_cast<unsigned int>(wWidth), static_cast<unsigned int>(wHeight)}),
        "Assignment 2");
    m_window.setKeyRepeatEnabled(false);
    m_window.setFramerateLimit(fps);

    if (!ImGui::SFML::Init(m_window))
    {
        std::cerr << "Could not init window!\n";
        return;
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

        m_currentFrame++;
    }
}

void Game::spawnPlayer()
{
    auto e = m_entities.addEntity("player");
    e->add<CTransform>(Vec2<float>(200.0f, 200.0f), Vec2<float>(0.f, 0.f), 0.0f);
    e->add<CShape>(m_playerConfig.SR, m_playerConfig.V,
                   sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
                   sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
                   m_playerConfig.OT);
    e->add<CInput>();
}

void Game::spawnEnemy()
{
    int rand_pts = randInt(m_enemyConfig.VMIN, m_enemyConfig.VMAX);

    auto size = m_window.getSize();
    float x = randFloat(m_enemyConfig.SR, size.x - m_enemyConfig.SR);
    float y = randFloat(m_enemyConfig.SR, size.y - m_enemyConfig.SR);
    Vec2<float> pos(x, y);

    float vx = randFloat(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
    float vy = randFloat(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
    Vec2<float> velocity(vx, vy);

    std::uniform_int_distribution<int> col(1, 255);
    sf::Color randomFill(col(m_rng), col(m_rng), col(m_rng));

    auto e = m_entities.addEntity("enemy");
    e->add<CTransform>(pos, velocity, 0.0f);
    e->add<CShape>(m_enemyConfig.SR, rand_pts, randomFill,
                   sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
                   m_enemyConfig.OT);

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    // TODO: spawn small enemies at the location of the input enemy e
    Vec2<float> spawnLocation = e->get<CTransform>().pos;
    auto parentFillCol = e->get<CShape>().circle.getFillColor();
    auto parentOutlineCol = e->get<CShape>().circle.getOutlineColor();
    int parentPointCount = e->get<CShape>().circle.getPointCount();

    float vx = randFloat(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
    float vy = randFloat(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
    Vec2<float> velocity(vx, vy);

    for (int i = 0; i < parentPointCount; i++)
    {
        auto s = m_entities.addEntity("smallEnemy");
        s->add<CTransform>(spawnLocation, velocity, 0.0f);
        s->add<CShape>(16, parentPointCount,
             parentFillCol,
             parentOutlineCol,
             m_playerConfig.OT);
    }

    // when we create the smaller enemy, we have to read the values of the
    // original enemy
    // - spawn a number of small enemies equal to the vertices of the original
    // enemy
    // - set each small enemy to the same color as the original, half the size
    // - small enemies are worth double points of the original enemy
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2<float> &target)
{
    // TODO: implement the spawning of a bullet which travels toward target
    //       -- bullet speed is given as a scalar speed
    //       -- you must set the velocity by using formula in notes

    Vec2<float> dir = target - entity->get<CTransform>().pos;
    dir.normalize();
    float speed = m_bulletConfig.S;
    Vec2<float> velocity = dir * speed;

    auto spawnPos = entity->get<CTransform>().pos;

    auto b = m_entities.addEntity("bullet");
    b->add<CTransform>(spawnPos, velocity, 0.0f);
    b->add<CShape>(m_bulletConfig.SR, m_bulletConfig.V,
                sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
                sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
                m_bulletConfig.OT);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // TODO: implement special weapon
}

void Game::sMovement()
{
    auto &pTransform = player()->get<CTransform>();
    auto &pInput = player()->get<CInput>();

    Vec2<float> dir(0.0f, 0.0f);

    if (pInput.up) dir.y -= 1.f;
    if (pInput.down) dir.y += 1.f;
    if (pInput.right) dir.x += 1.f;
    if (pInput.left) dir.x -= 1.f;

    if (dir.x != 0.f || dir.y != 0.f)
    {
        dir.normalize();
        pTransform.velocity = dir * m_playerConfig.S;
    }
    else 
    {
        pTransform.velocity = {0.f, 0.f};
    }
    
    for (auto &e : m_entities.getEntities())
    {
        if (!e->has<CTransform>()) continue;
        auto &t = e->get<CTransform>();
        t.pos += t.velocity;
    }
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

    for (auto &e : m_entities.getEntities())
    {
        if (e->has<CShape>() && e->has<CTransform>())
        {
            auto &shape = e->get<CShape>();
            auto &transform = e->get<CTransform>();
            
            shape.circle.setPosition(transform.pos);

            m_window.draw(shape.circle);
        }
    }

    // draw ui last
    ImGui::SFML::Render(m_window);

    m_window.display();
}

void Game::sUserInput()
{
    while (auto event = m_window.pollEvent())
    {
        // pass the event to imgui to be parsed
        ImGui::SFML::ProcessEvent(m_window, *event);

        auto &pInput = player()->get<CInput>();

        if (event->is<sf::Event::Closed>())
        {
            std::exit(0);
        }

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            std::cout << "Key pressed = " << int(keyPressed->scancode) << "\n";

            if (keyPressed->scancode == sf::Keyboard::Scancode::W) pInput.up = true;
            if (keyPressed->scancode == sf::Keyboard::Scancode::S) pInput.down = true;
            if (keyPressed->scancode == sf::Keyboard::Scancode::A) pInput.left = true;
            if (keyPressed->scancode == sf::Keyboard::Scancode::D) pInput.right = true;
        }

        if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            if (keyReleased->scancode == sf::Keyboard::Scancode::W) pInput.up = false;
            if (keyReleased->scancode == sf::Keyboard::Scancode::S) pInput.down = false;
            if (keyReleased->scancode == sf::Keyboard::Scancode::A) pInput.left = false;
            if (keyReleased->scancode == sf::Keyboard::Scancode::D) pInput.right = false;
        }

        if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            Vec2<float> mpos(mousePressed->position);

            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                spawnBullet(player(), mpos);
            }
            else if (mousePressed->button == sf::Mouse::Button::Right)
            {
                // TODO: call special weapon here
                std::cout << "Right mouse (" << mpos.x << ", " << mpos.y << ")\n";
            }
        }
    }
}

int Game::randInt(int min, int max)
{
    std::uniform_int_distribution<int> num(min, max);
    return num(m_rng);
}

float Game::randFloat(int min, int max)
{
    std::uniform_real_distribution<float> num(min, max);
    return num(m_rng);
}
