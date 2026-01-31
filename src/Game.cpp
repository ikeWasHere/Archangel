#include "Game.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <cstdint>

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
        "Geometry Wars");
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
        ImGui::SFML::Update(m_window, dtTime);

        if (m_systems.input) sUserInput();
        if (m_systems.spawner) sEnemySpawner();
        if (m_systems.movement) sMovement(dt);
        if (m_systems.lifespan) sLifespan();
        if (m_systems.collision) sCollision();
        sGUI();
        if (m_systems.render) sRender();
    
        m_currentFrame++;
    }
}

void Game::spawnPlayer()
{
    auto size = m_window.getSize();
    float spawnX = size.x * 0.5;
    float spawnY = size.y * 0.5;
    float angVel = 180.f;

    auto e = m_entities.addEntity("player");
    e->add<CTransform>(Vec2<float>(spawnX, spawnY), Vec2<float>(0.f, 0.f), 0.0f, angVel);
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

    float angVel = randFloat(-180.f, 180.f);
    if (std::abs(angVel) < 30.f)
        angVel = (angVel < 0 ? -30.f : 30.f);

    std::uniform_int_distribution<int> col(1, 255);
    sf::Color randomFill(col(m_rng), col(m_rng), col(m_rng));

    auto e = m_entities.addEntity("enemy");
    e->add<CTransform>(pos, velocity, 0.0f, angVel);
    e->add<CShape>(m_enemyConfig.SR, rand_pts, randomFill,
                   sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
                   m_enemyConfig.OT);
    e->add<CCollision>(m_enemyConfig.CR);

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    Vec2<float> spawnLocation = e->get<CTransform>().pos;
    float angVel = randFloat(-180.f, 180.f);
    if (std::abs(angVel) < 30.f)
        angVel = (angVel < 0 ? -30.f : 30.f);

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
        s->add<CTransform>(spawnLocation, velocity, 0.0f, angVel);
        s->add<CShape>(m_enemyConfig.SR / 2, parentPointCount,
             parentFillCol,
             parentOutlineCol,
             m_enemyConfig.OT);
        s->add<CCollision>(m_enemyConfig.CR / 2);
        s->add<CLifespan>(m_enemyConfig.L);
    }
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2<float> &target)
{
    Vec2<float> dir = target - entity->get<CTransform>().pos;
    dir.normalize();
    float speed = m_bulletConfig.S;
    Vec2<float> velocity = dir * speed;

    auto spawnPos = entity->get<CTransform>().pos;

    auto b = m_entities.addEntity("bullet");
    b->add<CTransform>(spawnPos, velocity, 0.0f, 0.0f);
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
        t.angle += t.angVel * dt;
    }
}

void Game::sLifespan()
{
    for (auto &e : m_entities.getEntities())
    {
        if (!e->has<CLifespan>()) continue;
        
        auto &life = e->get<CLifespan>();
  
        if (life.remaining > 0)
        {
            life.remaining -= 1;
        }

        // Fade alpha 1:1 with remaining lifespan.
        if (e->has<CShape>())
        {
            auto &circle = e->get<CShape>().circle;
            const float ratio = std::clamp(life.remaining / static_cast<float>(life.lifespan), 0.f, 1.f);
            auto applyAlpha = [ratio](sf::Color c)
            {
                c.a = static_cast<std::uint8_t>(ratio * 255.0f);
                return c;
            };
            circle.setFillColor(applyAlpha(circle.getFillColor()));
            circle.setOutlineColor(applyAlpha(circle.getOutlineColor()));
        }

        if (life.remaining <= 0)
        {
            e->destroy();
        }
    }
}

void Game::sCollision()
{
    int &pScore = player()->get<CScore>().score;
    auto size = m_window.getSize();
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

    // Collisions with walls
    float w = static_cast<float>(size.x);
    float h = static_cast<float>(size.y);

    for (auto e : m_entities.getEntities())
    {
        if (!e->has<CCollision>() || !e->has<CTransform>()) continue;

        auto &t = e->get<CTransform>();
        auto &c = e->get<CCollision>();
        float r = c.radius;

        bool bouncedX = false;
        bool bouncedY = false;

        // Left wall
        if (t.pos.x - r < 0.f)
        {
            t.pos.x = r;
            bouncedX = true;
        }
        else if (t.pos.x + r > w)
        {
            t.pos.x = w - r;
            bouncedX = true;
        }

        // Top wall
        if (t.pos.y - r < 0.f)
        {
            t.pos.y = r;
            bouncedY = true;
        }
        else if (t.pos.y + r > h)
        {
            t.pos.y = h - r;
            bouncedY = true;
        }

        if (bouncedX) { t.velocity.x *= -1.f; }
        if (bouncedY) { t.velocity.y *= -1.f; }
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
    if (ImGui::BeginTabBar("Geometry Wars TabBar"))
    {
        if (ImGui::BeginTabItem("Systems"))
        {
            ImGui::Checkbox("User Input", &m_systems.input);
            ImGui::Checkbox("Enemy Spawner", &m_systems.spawner);
            ImGui::Checkbox("Movement", &m_systems.movement);
            ImGui::Checkbox("Lifespan", &m_systems.lifespan);
            ImGui::Checkbox("Collision", &m_systems.collision);
            ImGui::Checkbox("Render", &m_systems.render);
            
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entity Manager"))
        {
            auto drawEntityRow = [](std::shared_ptr<Entity> e)
            {
                ImGui::PushID(static_cast<int>(e->id()));

                // Color preview (fill color if available)
                sf::Color preview = e->has<CShape>() ? e->get<CShape>().circle.getFillColor()
                                                     : sf::Color(128, 128, 128);
                ImVec4 imguiCol(preview.r / 255.f, preview.g / 255.f, preview.b / 255.f, preview.a / 255.f);
                ImGui::ColorButton("##color", imguiCol, ImGuiColorEditFlags_NoTooltip, ImVec2(18, 18));
                ImGui::SameLine();
                
                // Destroy toggle
                if (ImGui::Button("D", ImVec2(20, 20)))
                {
                    e->destroy();
                }
                ImGui::SameLine();

                ImGui::Text("%zu  %s", e->id(), e->tag().c_str());
                if (e->has<CTransform>())
                {
                    auto &t = e->get<CTransform>();
                    ImGui::SameLine();
                    ImGui::Text("(%.0f,%.0f)", t.pos.x, t.pos.y);
                }

                ImGui::PopID();
            };

            if (ImGui::CollapsingHeader("Entities by Tag", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const auto &entityMap = m_entities.getEntityMap();
                for (const auto &[tag, vec] : entityMap)
                {
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
                    if (ImGui::TreeNodeEx(tag.c_str(), flags))
                    {
                        for (auto &e : vec)
                        {
                            drawEntityRow(e);
                        }
                        ImGui::TreePop();
                    }
                }
            }

            if (ImGui::CollapsingHeader("All Entities"))
            {
                for (auto &e : m_entities.getEntities())
                {
                    drawEntityRow(e);
                }
            }
            
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void Game::sRender()
{
    if (!m_window.isOpen()) return;
    
    m_window.clear();

    for (auto &e : m_entities.getEntities())
    {
        if (e->has<CShape>() && e->has<CTransform>())
        {
            auto &shape = e->get<CShape>();
            auto &transform = e->get<CTransform>();
            
            shape.circle.setPosition(transform.pos);
            shape.circle.setRotation(sf::degrees(transform.angle));
           
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
