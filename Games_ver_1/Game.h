#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

constexpr int WINDOW_WIDTH = 1920;  // 窗口宽度
constexpr int WINDOW_HEIGHT = 1080; // 窗口高度
constexpr float PLAYER_RADIUS = 25.f;   // 玩家半径
constexpr float ENEMY_RADIUS = 25.f;    // 敌方半径
constexpr float REBOUND_COEFFICIENT = 0.8f; // 反弹系数
constexpr float FRICTION_COEFFICIENT = 0.98f;   // 摩擦系数
const sf::Vector2f CENTER_ZONE_POSITION(710, 290);  // 中心区域左上坐标
const sf::Vector2f CENTER_ZONE_SIZE(500, 500);      // 中心区域大小
constexpr int NUM_ENEMIES = 6;  // 敌方数量
constexpr float CHARGE_MAX_TIME = 4.f;  // 蓄力时间

class TextureManager {
public:
    sf::Texture& getTexture(const std::string& filename) {
        auto& texture = textures[filename];
        if (!texture.loadFromFile(filename)) {
            std::cerr << "Error loading texture from " << filename << std::endl;
            throw std::runtime_error("Failed to load texture!");
        }
        return texture;
    }

private:
    std::map<std::string, sf::Texture> textures;  // Store textures
};

// Usage in GameObject
class GameObject {
public:
    sf::Sprite sprite;
    sf::Vector2f velocity;  // Velocity
    float mass;  // Mass property
    bool isStopped;

    GameObject(float radius, const std::string& textureFile, sf::Vector2f position, TextureManager& textureManager, float m = 1.0f)
            : velocity(0.f, 0.f), mass(m), isStopped(true) {
        sprite.setTexture(textureManager.getTexture(textureFile));
        sprite.setOrigin(radius, radius);
        sprite.setPosition(position);
        sprite.setScale(radius * 2 / (float)sprite.getTexture()->getSize().x, radius * 2 / (float)sprite.getTexture()->getSize().y);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);  // Draw the sprite
    }

    void updatePosition(float dt) {
        if (!isStopped) {
            sprite.move(velocity * dt);
            velocity *= FRICTION_COEFFICIENT; // 摩擦减速
            if (std::abs(velocity.x) < 0.01f && std::abs(velocity.y) < 0.01f) {
                isStopped = true;
                velocity = {0.f, 0.f};
            }
        }
    }

    void applyBoundaryCollision() {
        auto bounds = sprite.getGlobalBounds();  // Get the global bounds of the sprite

        // Check for left and right boundary collisions
        if (bounds.left < 280 || bounds.left + bounds.width > WINDOW_WIDTH - 300) {
            velocity.x = -velocity.x * REBOUND_COEFFICIENT;  // Reverse horizontal velocity
            sprite.setPosition(std::max(bounds.width / 2, std::min(sprite.getPosition().x, WINDOW_WIDTH - bounds.width / 2)), sprite.getPosition().y);  // Keep sprite within bounds
        }

        // Check for top and bottom boundary collisions
        if (bounds.top < 120 || bounds.top + bounds.height > WINDOW_HEIGHT) {
            velocity.y = -velocity.y * REBOUND_COEFFICIENT;  // Reverse vertical velocity
            sprite.setPosition(sprite.getPosition().x, std::max(bounds.height / 2, std::min(sprite.getPosition().y, WINDOW_HEIGHT - bounds.height / 2)));  // Keep sprite within bounds
        }
    }
    void save(std::ofstream& file) const {
        file.write(reinterpret_cast<const char*>(&sprite.getPosition().x), sizeof(float));
        file.write(reinterpret_cast<const char*>(&sprite.getPosition().y), sizeof(float));
        file.write(reinterpret_cast<const char*>(&velocity.x), sizeof(float));
        file.write(reinterpret_cast<const char*>(&velocity.y), sizeof(float));
        file.write(reinterpret_cast<const char*>(&mass), sizeof(float));
        file.write(reinterpret_cast<const char*>(&isStopped), sizeof(bool));
    }

    void load(std::ifstream& file) {
        float x, y, vx, vy, m;
        bool stopped;
        file.read(reinterpret_cast<char*>(&x), sizeof(float));
        file.read(reinterpret_cast<char*>(&y), sizeof(float));
        file.read(reinterpret_cast<char*>(&vx), sizeof(float));
        file.read(reinterpret_cast<char*>(&vy), sizeof(float));
        file.read(reinterpret_cast<char*>(&m), sizeof(float));
        file.read(reinterpret_cast<char*>(&stopped), sizeof(bool));

        sprite.setPosition(x, y);
        velocity.x = vx;
        velocity.y = vy;
        mass = m;
        isStopped = stopped;
    }
};

class CollisionHandler {
public:static void applyCollision(GameObject& a, GameObject& b, sf::Sound& collisionSound) {
        // Get the bounding boxes of the sprites
        sf::FloatRect aBounds = a.sprite.getGlobalBounds();
        sf::FloatRect bBounds = b.sprite.getGlobalBounds();

        // Check for collision using bounding boxes
        if (aBounds.intersects(bBounds)) {
            collisionSound.play();

            auto delta = a.sprite.getPosition() - b.sprite.getPosition();  // Update for sprite
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

            // Calculate the normal and relative velocity
            auto normal = delta / dist;  // Collision normal
            auto relativeVelocity = a.velocity - b.velocity;
            float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

            if (velocityAlongNormal > 0) return;  // Moving apart

            // Update velocities
            sf::Vector2f temp = a.velocity;
            a.velocity = 0.5f * (a.velocity + b.velocity + REBOUND_COEFFICIENT * (b.velocity - a.velocity));
            b.velocity = 0.5f * (b.velocity + temp + REBOUND_COEFFICIENT * (temp - b.velocity));

            a.isStopped = false;
            b.isStopped = false;

            // Debug output
            //std::cout << "Collision detected: a velocity: " << a.velocity.x << ", " << a.velocity.y
            //          << " | b velocity: " << b.velocity.x << ", " << b.velocity.y << std::endl;
        }
    }
};


class ScoreManager {
private:
    int currentScore;
    int highScore;
    std::string filename;

public:
    ScoreManager(const std::string& file) : currentScore(0), filename(file) {
        highScore = findHighScore();
    }

    int getCurrentScore() const { return currentScore; }

    void updateScore(int newscore) {
        currentScore = newscore;
        updateHighScore();
    }

    void saveScore() {
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << currentScore << "\n";
            file.close();
        }
    }

    int findHighScore() {
        std::ifstream file(filename);
        int highestScore = 0, score;
        if (file.is_open()) {
            while (file >> score) {
                highestScore = std::max(highestScore, score);
            }
            file.close();
        }
        return highestScore;
    }

    void updateHighScore() {
        if (currentScore > highScore) {
            highScore = currentScore;
        }
    }

    int getHighScore() const { return highScore; }

    int& getCurrentScoreRef() { return currentScore; } // Correctly added
    const int& getCurrentScoreRef() const { return currentScore; } // const version
};


class Game {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text scoreText, highScoreText, finalScoreText, playerCountText, selectionText;
    sf::RectangleShape chargeBar;
    sf::RectangleShape centerZoneBorder;
    sf::SoundBuffer collisionBuffer;
    sf::Sound collisionSound;
    TextureManager textureManager;

    ScoreManager scoreManager;
    std::vector<GameObject> enemies;
    std::vector<GameObject> players;

    int normalCount;
    int specialCount;
    bool isCharging;
    float chargeTime;
    int selectedPlayerIndex;
    int hadshoot;


    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    sf::Texture backgroundTextureEnd;
    sf::Sprite backgroundSpriteEnd;

    bool viewArchiveMode;
    sf::Music backgroundMusic;
    bool allPlayersStopped;

public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Game"),
             scoreManager("highscores.txt"),
             normalCount(2), specialCount(2), isCharging(false), chargeTime(0.f),
             selectedPlayerIndex(-1), hadshoot(0), viewArchiveMode(false) {  // 默认没有选择
        window.setFramerateLimit(60);

        // 加载背景图片
        if (!backgroundTexture.loadFromFile("Images/background.png")) {
            std::cerr << "Failed to load background image!" << std::endl;
        } else {
            backgroundSprite.setTexture(backgroundTexture);
        }

        if (!backgroundTextureEnd.loadFromFile("Images/backgroundend.png")) {
            std::cerr << "Failed to load background image!" << std::endl;
        } else {
            backgroundSpriteEnd.setTexture(backgroundTextureEnd);
        }

        // 加载字体，并检查加载是否成功
        if (!font.loadFromFile("chinese.ttf")) {
            std::cerr << "Failed to load font: chinese.ttf" << std::endl;
            // 处理字体加载失败的情况，例如使用默认字体或退出程序
            return; // 这里选择直接退出构造函数
        }

        // 加载并播放背景音乐
        if (!backgroundMusic.openFromFile("background_music.flac")) {
            std::cerr << "Failed to load background music!" << std::endl;
        } else {
            backgroundMusic.setLoop(true); // 设置循环播放
            backgroundMusic.setVolume(50.f); // 设置音量 (0.0f - 100.0f)
            backgroundMusic.play(); // 开始播放
        }

        initializeText(scoreText, 30, sf::Vector2f(WINDOW_WIDTH - 600, 140));
        initializeText(highScoreText, 30, sf::Vector2f(WINDOW_WIDTH - 600, 185));
        initializeText(playerCountText, 30, sf::Vector2f(WINDOW_WIDTH - 550, WINDOW_HEIGHT - 220));
        initializeText(selectionText, 24, sf::Vector2f(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT - 150));

        centerZoneBorder.setSize(CENTER_ZONE_SIZE);
        centerZoneBorder.setOutlineThickness(30);
        centerZoneBorder.setOutlineColor(sf::Color::Green);
        centerZoneBorder.setFillColor(sf::Color::Transparent);
        centerZoneBorder.setPosition(CENTER_ZONE_POSITION);

        chargeBar.setSize(sf::Vector2f(30, 200));
        chargeBar.setFillColor(sf::Color::Yellow);
        chargeBar.setPosition(WINDOW_WIDTH - 180, WINDOW_HEIGHT - 620);

        collisionBuffer.loadFromFile("collision.flac");
        collisionSound.setBuffer(collisionBuffer);

        initializeEnemies();
        initializePlayers();
    }


    void run() {
        while (window.isOpen()) {
            handleEvents();

            if (viewArchiveMode) {
                loadGame("savegame.bin");
                render();
                continue;
            }

            if (isCharging) updateCharge();
            updateGameObjects();
            checkCollisions();
            updateEnemyCount();
            updateMessage();

            // 检查所有球是否停止
            allPlayersStopped = std::all_of(players.begin(), players.end(), [](const GameObject& player) {
                return player.isStopped;
            });

            if (hadshoot >= players.size() && allPlayersStopped) {
                renderEndScene(); // 所有球发射完毕且停止后才进入结束画面
            } else {
                render();
            }
        }
        scoreManager.saveScore();
    }


private:
    void initializeText(sf::Text &text, int size, sf::Vector2f position) {
        text.setFont(font);
        text.setCharacterSize(size);
        text.setPosition(position);
        text.setFillColor(sf::Color::White);
    }

    void initializeEnemies() {
        for (int i = 0; i < NUM_ENEMIES; ++i) {
            sf::Vector2f position;
            bool validPosition = false;

            while (!validPosition) {
                float x = CENTER_ZONE_POSITION.x + std::rand() % static_cast<int>(CENTER_ZONE_SIZE.x);
                float y = CENTER_ZONE_POSITION.y + std::rand() % static_cast<int>(CENTER_ZONE_SIZE.y);
                position = sf::Vector2f(x, y);
                validPosition = true;

                if (position.x - ENEMY_RADIUS < CENTER_ZONE_POSITION.x ||
                    position.x + ENEMY_RADIUS > CENTER_ZONE_POSITION.x + CENTER_ZONE_SIZE.x ||
                    position.y - ENEMY_RADIUS < CENTER_ZONE_POSITION.y ||
                    position.y + ENEMY_RADIUS > CENTER_ZONE_POSITION.y + CENTER_ZONE_SIZE.y) {
                    validPosition = false;
                    continue;
                }

                for (const auto &enemy: enemies) {
                    float dist = std::sqrt(std::pow(position.x - enemy.sprite.getPosition().x, 2) +
                                           std::pow(position.y - enemy.sprite.getPosition().y, 2));
                    if (dist < ENEMY_RADIUS + ENEMY_RADIUS) {
                        validPosition = false;
                        break;
                    }
                }
            }
            enemies.emplace_back(ENEMY_RADIUS, "Images/bird_2.png", position, textureManager);
        }
    }

    void initializePlayers() {
        selectedPlayerIndex = 0;
        players = {
                GameObject(PLAYER_RADIUS, "Images/bird_1.png",
                           sf::Vector2f(WINDOW_WIDTH / 2 - 170, WINDOW_HEIGHT - 170), textureManager),
                GameObject(PLAYER_RADIUS, "Images/bird_1.png", sf::Vector2f(WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT - 170),
                           textureManager),
                GameObject(PLAYER_RADIUS, "Images/bird_1.png", sf::Vector2f(WINDOW_WIDTH / 2 + 30, WINDOW_HEIGHT - 170),
                           textureManager),
                GameObject(PLAYER_RADIUS, "Images/bird_1.png",
                           sf::Vector2f(WINDOW_WIDTH / 2 + 130, WINDOW_HEIGHT - 170), textureManager)
        };
    }


    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // 处理存档模式下的事件
            if (viewArchiveMode) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    viewArchiveMode = false; // 按 Esc 退出存档模式
                }
                continue; // 存档模式下不再处理其他事件
            }

            // 处理正常游戏模式下的事件
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::V && hadshoot >= players.size()) {
                    viewArchiveMode = true; // 游戏结束后按 V 进入存档模式
                }
                if (event.key.code == sf::Keyboard::Num1) selectedPlayerIndex = 0;
                if (event.key.code == sf::Keyboard::Num2) selectedPlayerIndex = 1;
                if (event.key.code == sf::Keyboard::Num3) selectedPlayerIndex = 2;
                if (event.key.code == sf::Keyboard::Num4) selectedPlayerIndex = 3;
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && hadshoot < players.size()) {
                if (selectedPlayerIndex != -1) {
                    isCharging = true;
                    chargeTime = 0.f;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && isCharging) {
                if (hadshoot < players.size()) {
                    selectedPlayerIndex = hadshoot;
                    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
                    launchPlayer(mousePos);
                    isCharging = false;
                    hadshoot++;
                }
            }
        }
    }

    void updateCharge() {
        if (isCharging && hadshoot < players.size()) { //添加hadshoot判断
            chargeTime += 0.016f;
            if (chargeTime > CHARGE_MAX_TIME) {
                chargeTime = CHARGE_MAX_TIME;
            }
            chargeBar.setSize(sf::Vector2f(30, chargeTime / CHARGE_MAX_TIME * 200));
        }
    }


    void launchPlayer(const sf::Vector2f &mousePos) {
        if (selectedPlayerIndex >= 0 && selectedPlayerIndex < players.size() &&
            hadshoot < players.size()){ //添加越界检查
                auto &player = players[selectedPlayerIndex];
                // 计算鼠标相对于棋子位置的方向
                sf::Vector2f direction = mousePos - player.sprite.getPosition();
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);  // 获取方向长度

                // 归一化方向向量, 避免除以零
                if (length > 0) {
                    direction /= length;  // 归一化
                }

                // 根据蓄力时间计算发射速度
                player.velocity = direction * (chargeTime / CHARGE_MAX_TIME * 250.f);
                player.isStopped = false;  // 使棋子开始移动
            }
        }

    void updateGameObjects() {
        for (auto& enemy : enemies) enemy.updatePosition(0.1f);
        for (auto& player : players) player.updatePosition(0.1f);

        if (hadshoot < players.size()) { // 只有在还有剩余次数时才更新分数和碰撞
            updateEnemyCount();
            checkCollisions();
        }
    }

        void updateEnemyCount() {
            int count = 0;

            for (const auto &enemy: enemies) {
                auto bounds = enemy.sprite.getGlobalBounds();

                // 检查敌人是否在中心区域外
                if (bounds.left + bounds.width <= CENTER_ZONE_POSITION.x ||  // 左侧完全在中心区域外
                    bounds.left >= CENTER_ZONE_POSITION.x + CENTER_ZONE_SIZE.x || // 右侧完全在中心区域外
                    bounds.top + bounds.height <= CENTER_ZONE_POSITION.y ||  // 上侧完全在中心区域外
                    bounds.top >= CENTER_ZONE_POSITION.y + CENTER_ZONE_SIZE.y) { // 下侧完全在中心区域外
                    count++;
                }
            }

            scoreManager.updateScore(count);

            // 更新分数文本
            scoreText.setString(L"本局分数： " + std::to_wstring(scoreManager.getCurrentScore()));
        }

        //确定文字信息
        void updateMessage() {
            highScoreText.setString(L"历史记录： " + std::to_wstring(scoreManager.getHighScore()));
            playerCountText.setString(L"剩余次数： " + std::to_wstring(std::max(0, (int)players.size() - hadshoot)));
        }


        void checkCollisions() {
            for (auto &player: players) player.applyBoundaryCollision();
            for (auto &enemy: enemies) enemy.applyBoundaryCollision();

            // 检测每个player与每个enemy的碰撞
            for (size_t i = 0; i < players.size(); ++i) {
                for (size_t j = 0; j < enemies.size(); ++j) {
                    CollisionHandler::applyCollision(players[i], enemies[j], collisionSound);
                }
            }

            // 可选：如果需要检测 players 之间的碰撞，可以再加一个循环
            for (size_t i = 0; i < players.size(); ++i) {
                for (size_t j = i + 1; j < players.size(); ++j) {
                    CollisionHandler::applyCollision(players[i], players[j], collisionSound);
                }
            }

            for (size_t i = 0; i < enemies.size(); ++i) {
                for (size_t j = i + 1; j < enemies.size(); ++j) {
                    CollisionHandler::applyCollision(enemies[i], enemies[j], collisionSound);
                }
            }
        }

        // 修改 render() 方法以绘制精灵
        void render() {
            window.clear();

            // 绘制背景图片
            window.draw(backgroundSprite);

            window.draw(centerZoneBorder);
            window.draw(scoreText);
            window.draw(highScoreText);
            window.draw(playerCountText);
            window.draw(chargeBar);

            for (const auto &enemy: enemies) enemy.draw(window);  // 调用 draw 方法
            for (const auto &player: players) player.draw(window);  // 调用 draw 方法

            window.draw(selectionText);  // 渲染选择文本
            window.display();


        }

    void renderEndScene() {
        window.clear();
        window.draw(backgroundSpriteEnd);

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::V) {
                loadGame("savegame.bin");
                viewArchiveMode = true;
                //这里需要添加一个return，不然会继续执行下面的渲染代码，导致存档模式的画面和结束画面重叠
                return;
            }
        }

        window.clear();
        window.draw(backgroundSpriteEnd);

        sf::Text gameOverHighScore;
        sf::Text gameOverCurrentScore;
        sf::Text archiveView;
        sf::Text remainingShots;

        remainingShots.setFont(font);
        remainingShots.setString(L"剩余次数： " + std::to_wstring(std::max(0, (int)players.size() - hadshoot)));
        remainingShots.setCharacterSize(40);
        remainingShots.setFillColor(sf::Color::White);
        remainingShots.setPosition(WINDOW_WIDTH / 2 - remainingShots.getGlobalBounds().width / 2, WINDOW_HEIGHT - 150);

        gameOverHighScore.setFont(font);
        gameOverHighScore.setString(L"历史记录： " + std::to_wstring(scoreManager.getHighScore()));
        gameOverHighScore.setCharacterSize(70);
        gameOverHighScore.setFillColor(sf::Color::White);
        gameOverHighScore.setPosition(WINDOW_WIDTH / 2 - gameOverHighScore.getGlobalBounds().width / 2, WINDOW_HEIGHT / 2 - gameOverHighScore.getGlobalBounds().height / 2 + 40);

        gameOverCurrentScore.setFont(font);
        gameOverCurrentScore.setString(L"本局分数： " + std::to_wstring(scoreManager.getCurrentScore()));
        gameOverCurrentScore.setCharacterSize(70);
        gameOverCurrentScore.setFillColor(sf::Color::White);
        gameOverCurrentScore.setPosition(WINDOW_WIDTH / 2 - gameOverCurrentScore.getGlobalBounds().width / 2, WINDOW_HEIGHT / 2 - gameOverCurrentScore.getGlobalBounds().height / 2 - 100);

        archiveView.setFont(font);
        archiveView.setString(L"按 V 键查看存档");
        archiveView.setCharacterSize(40);
        archiveView.setFillColor(sf::Color::White);
        archiveView.setPosition(WINDOW_WIDTH / 2 - archiveView.getGlobalBounds().width / 2, WINDOW_HEIGHT - 200);

        window.draw(gameOverHighScore);
        window.draw(gameOverCurrentScore);
        window.draw(archiveView);
        window.draw(remainingShots);

        window.display();
    }

        void saveGame(const std::string &filename) { // Now correctly inside the class
            std::ofstream file(filename, std::ios::binary);
            if (file.is_open()) {
                file.write(reinterpret_cast<const char *>(&hadshoot), sizeof(int));
                file.write(reinterpret_cast<const char *>(&scoreManager.getCurrentScoreRef()), sizeof(int));

                int enemyCount = enemies.size();
                file.write(reinterpret_cast<const char *>(&enemyCount), sizeof(int));
                for (const auto &enemy: enemies) {
                    enemy.save(file);
                }

                int playerCount = players.size();
                file.write(reinterpret_cast<const char *>(&playerCount), sizeof(int));
                for (const auto &player: players) {
                    player.save(file);
                }
                file.close();
            } else {
                std::cerr << "Error saving game!" << std::endl;
            }
        }

        void loadGame(const std::string &filename) { // Now correctly inside the class
            std::ifstream file(filename, std::ios::binary);
            if (file.is_open()) {
                file.read(reinterpret_cast<char *>(&hadshoot), sizeof(int));
                file.read(reinterpret_cast<char *>(&scoreManager.getCurrentScoreRef()), sizeof(int));

                int enemyCount;
                file.read(reinterpret_cast<char *>(&enemyCount), sizeof(int));
                enemies.clear();
                for (int i = 0; i < enemyCount; ++i) {
                    enemies.emplace_back(ENEMY_RADIUS, "Images/bird_2.png", sf::Vector2f(0, 0), textureManager);
                    enemies.back().load(file);
                }

                int playerCount;
                file.read(reinterpret_cast<char *>(&playerCount), sizeof(int));
                players.clear();
                for (int i = 0; i < playerCount; ++i) {
                    players.emplace_back(PLAYER_RADIUS, "Images/bird_1.png", sf::Vector2f(0, 0), textureManager);
                    players.back().load(file);
                }
                file.close();
            } else {
                std::cerr << "Error loading game!" << std::endl;
            }
    }
};