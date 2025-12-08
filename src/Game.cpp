#include "Game.h"

#include <fstream>
#include <iostream>
#include <random>
#include <string>

Game::Game(const std::string &config)
    : m_text(m_font, "Defualt", 18)
{
    init(config);
}

Game::~Game()
{
    if (m_imguiInitialized)
    {
        ImGui::SFML::Shutdown();
    }
}

void Game::init(const std::string &path)
{
    std::string type;
    int wWidth = 1280;
    int wHeight = 720;
    int fps = 60;

    std::string fontPath;
    int fontSize = 18;

    bool windowCfgRead = false;
    bool fontCfgRead = false;
    bool playerCfgRead = false;
    bool enemyCfgRead = false;
    bool bulletCfgRead = false;

    std::ifstream inputFile(path);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Could not open the file!\n";
        return;
    }

    while (inputFile >> type)
    {
        if (type == "Window")
        {
            if (!(inputFile >> wWidth >> wHeight >> fps))
            {
                std::cerr << "Error: Malformed Window section in config\n";
                return;
            }
            windowCfgRead = true;
        }
        else if (type == "Font")
        {
            if (!(inputFile >> fontPath >> fontSize))
            {
                std::cerr << "Error: Malformed Font section in config\n";
                return;
            }
            fontCfgRead = true;
        }
        else if (type == "Player")
        {
            if (!(inputFile >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.FR >>
                  m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >>
                  m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V >> m_playerConfig.S))
            {
                std::cerr << "Error: Malformed Player section in config\n";
                return;
            }
            playerCfgRead = true;
        }
        else if (type == "Enemy")
        {
            if (!(inputFile >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.OR >> m_enemyConfig.OG >>
                  m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >>
                  m_enemyConfig.L >> m_enemyConfig.SI >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX))
            {
                std::cerr << "Error: Malformed Enemy section in config\n";
                return;
            }
            enemyCfgRead = true;
        }
        else if (type == "Bullet")
        {
            if (!(inputFile >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.FR >>
                  m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >>
                  m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L >>
                  m_bulletConfig.S))
            {
                std::cerr << "Error: Malformed Bullet section in config\n";
                return;
            }
            bulletCfgRead = true;
        }
        else
        {
            std::cerr << "Warning: Unknown config section '" << type << "'\n";
            std::string discard;
            std::getline(inputFile, discard);
        }
    }

    if (!(windowCfgRead && fontCfgRead && playerCfgRead && enemyCfgRead && bulletCfgRead))
    {
        std::cerr << "Error: Missing required config sections\n";
        return;
    }

    if (!m_font.openFromFile(fontPath))
    {
        std::cerr << "Error: Could not load font at " << fontPath << "\n";
        return;
    }
    m_text.setFont(m_font);
    m_text.setString("Default");
    m_text.setCharacterSize(static_cast<unsigned int>(fontSize));

    m_window.create(
        sf::VideoMode({static_cast<unsigned int>(wWidth), static_cast<unsigned int>(wHeight)}),
        "Assignment 2");
    m_window.setKeyRepeatEnabled(false);
    m_window.setFramerateLimit(fps);

    if (!ImGui::SFML::Init(m_window))
    {
        std::cerr << "Could not init window!\n";
        m_window.close();
        return;
    }

    m_imguiInitialized = true;
    m_configLoaded = true;
    spawnPlayer();
}

std::shared_ptr<Entity> Game::player()
{
    return m_entities.getEntities("player").back();
}

void Game::run()
{
    // TODO: add pause functionality in here
    m_paused = true;

    if (!m_configLoaded)
    {
        std::cerr << "Error: Cannot run game without a valid config\n";
        return;
    }

    while (m_window.isOpen())
    {
        sf::Time dtTime = m_deltaClock.restart();
        float dt = dtTime.asSeconds();
        m_entities.update();
        ImGui::SFML::Update(m_window,dtTime);

        sUserInput();
        sEnemySpawner();
        sMovement(dt);
        sLifespan();
        sCollision();
        sGUI();
        sRender();
    
       m_currentFrame++;
    }
}

void Game::spawnPlayer()
{
    auto size = m_window.getSize();
    float spawnX = size.x * 0.5;
    float spawnY = size.y * 0.5;

    auto e = m_entities.addEntity("player");
    e->add<CTransform>(Vec2<float>(spawnX, spawnY), Vec2<float>(0.f, 0.f), 0.0f);
    e->add<CShape>(m_playerConfig.SR, m_playerConfig.V,
                   sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
                   sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
                   m_playerConfig.OT);
    e->add<CCollision>(m_playerConfig.CR);
    e->add<CInput>();
    e->add<CScore>();
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
    e->add<CCollision>(m_enemyConfig.CR);

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    Vec2<float> spawnLocation = e->get<CTransform>().pos;
    auto parentFillCol = e->get<CShape>().circle.getFillColor();
    auto parentOutlineCol = e->get<CShape>().circle.getOutlineColor();
    // spawn a number of small enemies equal to the vertices of the original
    int parentPointCount = e->get<CShape>().circle.getPointCount();
    
    for (int i = 0; i < parentPointCount; i++)
    {
        float vx = randFloat(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
        float vy = randFloat(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
        Vec2<float> velocity(vx, vy);
        
        auto s = m_entities.addEntity("smallEnemy");
        s->add<CTransform>(spawnLocation, velocity, 0.0f);
        s->add<CShape>(m_enemyConfig.SR / 2, parentPointCount,
             parentFillCol,
             parentOutlineCol,
             m_enemyConfig.OT);
        s->add<CCollision>(m_enemyConfig.CR / 2);
        s->add<CLifespan>(m_enemyConfig.L);
    }
    // - small enemies are worth double points of the original enemy
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2<float> &target)
{
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
    b->add<CCollision>(m_bulletConfig.CR);
    b->add<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // TODO: implement special weapon
}

void Game::sMovement(float dt)
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
        t.pos += t.velocity * dt;
    }
    //std::cout << "Current pos: " << "(" << pTransform.pos.x << ", " << pTransform.pos.y << ")" << '\n';
    //std::cout << "Current dir: " << "(" << dir.x << ", " << dir.y << ")" << '\n';
    //std::cout << "Current vel: " << "(" << pTransform.velocity.x << ", " << pTransform.velocity.y << ")" << '\n';
    //std::cout << "Player speed (S): " << m_playerConfig.S << '\n';
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
    for (auto e : m_entities.getEntities())
    {
        if (!e->has<CLifespan>()) continue;
        auto &lifespan = e->get<CLifespan>().remaining;
        auto &fillColor = e->get<CShape>().circle;
        
        if (lifespan > 0) 
        {
            lifespan -= 1;
            //
        }
        
        else
        {
            e->destroy();
        }
   
    }
}

void Game::sCollision()
{
    int &pScore = player()->get<CScore>().score;
    int bigEnemyPoints = 25;
    int smallEnemyPoints = 50;
    
    for (auto b : m_entities.getEntities("bullet"))
    { 
        for (auto e : m_entities.getEntities("enemy"))
        {
           if (!b->has<CCollision>() || !e->has<CCollision>()) continue;

           if (isColliding(b, e))
           {
                b->destroy();
                e->destroy();
                spawnSmallEnemies(e);
                pScore += bigEnemyPoints;
           }
        }

        for (auto e : m_entities.getEntities("smallEnemy"))
        {
           if (!b->has<CCollision>() || !e->has<CCollision>()) continue;
           
           if (isColliding(b, e))
           {
                b->destroy();
                e->destroy();
                pScore += smallEnemyPoints;
           }
        }
    }

    // Player collisions
    for (auto e :m_entities.getEntities("enemy"))
    {
        if (!e->has<CCollision>()) continue;

        if (isColliding(player(), e))
        {
            respawnPlayer(player());
        }
    }

    for (auto e :m_entities.getEntities("smallEnemy"))
    {
        if (!e->has<CCollision>()) continue;

        if (isColliding(player(), e))
        {
            respawnPlayer(player());
        }
    }


}

void Game::sEnemySpawner()
{
    bool spawnNow = m_currentFrame % m_enemyConfig.SI == 0;

    if (spawnNow)
    {
        spawnEnemy();
    }
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
            m_window.close();
        }

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
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

float Game::randFloat(float min, float max)
{
    std::uniform_real_distribution<float> num(min, max);
    return num(m_rng);
}

bool Game::isColliding(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    auto &ta = a->get<CTransform>();
    auto &tb = b->get<CTransform>();

    auto &ca = a->get<CCollision>();
    auto &cb = b->get<CCollision>();

    float dx = ta.pos.x - tb.pos.x;
    float dy = ta.pos.y - tb.pos.y;

    float dist2 = dx * dx + dy * dy;
    float radiusSum = ca.radius + cb.radius;

    return dist2 <= radiusSum * radiusSum;
}

void Game::respawnPlayer(std::shared_ptr<Entity> player)
{
    auto size = m_window.getSize();
    float spawnX = size.x * 0.5;
    float spawnY = size.y * 0.5;

    auto &transform = player->get<CTransform>();
    transform.pos = Vec2<float>(spawnX, spawnY);
    transform.velocity = Vec2<float>(0.f, 0.f);
}
