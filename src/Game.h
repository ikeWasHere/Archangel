#pragma once

#include "Entity.hpp"
#include "EntityManager.hpp"

#include "imgui-SFML.h"
#include "imgui.h"
#include "imgui_stdlib.h"

#include <SFML/Graphics.hpp>
#include <random>

struct PlayerConfig
{
    int SR, CR, FR, FG, FB, OR, OG, OB, OT, V;
    float S;
};
struct EnemyConfig
{
    int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI;
    float SMIN, SMAX;
};
struct BulletConfig
{
    int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L;
    float S;
};

class Game
{
    sf::RenderWindow m_window;
    EntityManager m_entities;
    std::mt19937 m_rng{std::random_device{}()};
    sf::Font m_font;
    sf::Text m_text;
    PlayerConfig m_playerConfig{};
    EnemyConfig m_enemyConfig{};
    BulletConfig m_bulletConfig{};
    sf::Clock m_deltaClock;
    int m_score = 0;
    int m_currentFrame = 0;
    int m_lastEnemySpawnTime = 0;
    bool m_paused = false;
    bool m_configLoaded = false;
    bool m_imguiInitialized = false;
    
    struct SystemToggles
    {
        bool input = true;
        bool spawner = true;
        bool movement = true;
        bool lifespan = true;
        bool collision = true;
        bool render = true;
    } m_systems;

    void init(const std::string &config);
    void setPaused(bool paused);

    int randInt(int min, int max);
    float randFloat(float min, float max);

    void sMovement(float dt);
    void sUserInput();
    void sLifespan();
    void sRender();
    void sGUI();
    void sEnemySpawner();
    void sCollision();

    void spawnPlayer();
    void spawnEnemy();
    void spawnSmallEnemies(std::shared_ptr<Entity> entity);
    void spawnBullet(std::shared_ptr<Entity> entity, const Vec2<float> &mousePos);
    void spawnSpecialWeapon(std::shared_ptr<Entity> entity);
    bool isColliding(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    void respawnPlayer(std::shared_ptr<Entity> player);

    std::shared_ptr<Entity> player();

  public:
    Game(const std::string &config);
    ~Game();
    void run();
};
