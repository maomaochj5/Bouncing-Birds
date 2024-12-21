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
#include <cstring>

// 窗口相关常量
constexpr int WINDOW_WIDTH = 1920;      // 游戏窗口宽度（像素）
constexpr int WINDOW_HEIGHT = 1080;     // 游戏窗口高度（像素）

// 游戏对象尺寸常量
constexpr float PLAYER_RADIUS = 25.f;   // 玩家球体半径
constexpr float ENEMY_RADIUS = 25.f;    // 敌方球体半径

// 物理相关常量
constexpr float REBOUND_COEFFICIENT = 0.8f;    // 碰撞后的反弹系数（0-1之间，1为完全弹性碰撞）
constexpr float FRICTION_COEFFICIENT = 0.98f;   // 地面摩擦系数（每帧速度衰减比例）

// 中心区域（目标区域）相关常量
const sf::Vector2f CENTER_ZONE_POSITION(710, 290);  // 中心区域左上角坐标
const sf::Vector2f CENTER_ZONE_SIZE(500, 500);      // 中心区域的宽度和高度

// 游戏机制相关常量
constexpr int NUM_ENEMIES = 6;              // 场上敌方球体的数量
constexpr float CHARGE_MAX_TIME = 4.f;      // 最大蓄力时间（秒）

// 纹理管理器类：负责加载和管理所有游戏纹理
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
    // 存储容器
    std::map<std::string, sf::Texture> textures;  // 纹理映射表
};

// 游戏对象基类：所有可移动物体的基类
class GameObject {
public:
    // 视觉组件
    sf::Sprite sprite;                   // 精灵对象，用于渲染
    
    // 物理属性
    sf::Vector2f velocity;              // 速度向量
    float mass;                         // 质量
    bool isStopped;                     // 停止状态标志

    GameObject(float radius, const std::string& textureFile, sf::Vector2f position, TextureManager& textureManager, float m = 1.0f)
            : velocity(0.f, 0.f), mass(m), isStopped(true) {
        sprite.setTexture(textureManager.getTexture(textureFile));
        sprite.setOrigin(radius, radius);
        sprite.setPosition(position);
        sprite.setScale(radius * 2 / (float)sprite.getTexture()->getSize().x, radius * 2 / (float)sprite.getTexture()->getSize().y);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);  // 绘制精灵
    }

    void updatePosition(float dt) {
        if (!isStopped) {
            sprite.move(velocity * dt);
            velocity *= FRICTION_COEFFICIENT; // 应用摩擦力
            // 当速度足够小时停止物体
            if (std::abs(velocity.x) < 0.01f && std::abs(velocity.y) < 0.01f) {
                isStopped = true;
                velocity = {0.f, 0.f};
            }
        }
    }

    void applyBoundaryCollision() {
        auto bounds = sprite.getGlobalBounds();  // 获取精灵的边界框

        // 检测左右边界碰撞
        if (bounds.left < 280 || bounds.left + bounds.width > WINDOW_WIDTH - 300) {
            velocity.x = -velocity.x * REBOUND_COEFFICIENT;  // 水平速度反向
            sprite.setPosition(std::max(bounds.width / 2, std::min(sprite.getPosition().x, WINDOW_WIDTH - bounds.width / 2)), sprite.getPosition().y);
        }

        // 检测上下边界碰撞
        if (bounds.top < 120 || bounds.top + bounds.height > WINDOW_HEIGHT) {
            velocity.y = -velocity.y * REBOUND_COEFFICIENT;  // 垂直速度反向
            sprite.setPosition(sprite.getPosition().x, std::max(bounds.height / 2, std::min(sprite.getPosition().y, WINDOW_HEIGHT - bounds.height / 2)));
        }
    }
    void save(std::ofstream& file) const {
        std::vector<char> buffer(sizeof(float) * 4 + sizeof(bool) + sizeof(float));
        char* ptr = buffer.data();

        sf::Vector2f pos = sprite.getPosition(); // 创建非常量副本

        std::memcpy(ptr, &pos.x, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &pos.y, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &velocity.x, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &velocity.y, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &mass, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &isStopped, sizeof(bool));

        file.write(buffer.data(), buffer.size());
    }

    void load(std::ifstream& file) {
        std::vector<char> buffer(sizeof(float) * 4 + sizeof(bool) + sizeof(float));
        file.read(buffer.data(), buffer.size());

        char* ptr = buffer.data();

        sf::Vector2f pos; // 创建非常量Vector2f来接收数据
        std::memcpy(&pos.x, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&pos.y, ptr, sizeof(float));
        ptr += sizeof(float);
        sprite.setPosition(pos); 

        std::memcpy(&velocity.x, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&velocity.y, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&mass, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&isStopped, ptr, sizeof(bool));
    }
};

// 碰撞处理器类：处理游戏对象之间的碰撞
class CollisionHandler {
public:static void applyCollision(GameObject& a, GameObject& b, sf::Sound& collisionSound) {
        // 获取精灵的边界框
        sf::FloatRect aBounds = a.sprite.getGlobalBounds();
        sf::FloatRect bBounds = b.sprite.getGlobalBounds();

        // 使用边界框检查碰撞
        if (aBounds.intersects(bBounds)) {
            collisionSound.play();

            auto delta = a.sprite.getPosition() - b.sprite.getPosition();  // 更新精灵位置
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

            // 计算法线和相对速度
            auto normal = delta / dist;  // 碰撞法线
            auto relativeVelocity = a.velocity - b.velocity;
            float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

            if (velocityAlongNormal > 0) return;  // 物体正在分离

            // 更新速度
            sf::Vector2f temp = a.velocity;
            a.velocity = 0.5f * (a.velocity + b.velocity + REBOUND_COEFFICIENT * (b.velocity - a.velocity));
            b.velocity = 0.5f * (b.velocity + temp + REBOUND_COEFFICIENT * (temp - b.velocity));

            a.isStopped = false;
            b.isStopped = false;
        }
    }
};


class ScoreManager {
private:
    // 分数相关
    int currentScore;                   // 当前分数
    int highScore;                      // 最高分数
    std::string filename;               // 存档文件名

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

    int& getCurrentScoreRef() { return currentScore; } // 获取当前分数的引用
    const int& getCurrentScoreRef() const { return currentScore; } // 常量版本
};

// 游戏状态枚举：定义游戏的不同状态
enum GameState {
    Playing,                            // 游戏进行中
    EndScreen,                          // 结束画面
    ArchiveView                         // 存档查看
};

class Game {
private:
    // 窗口和渲染组件
    sf::RenderWindow window;                   // 主游戏窗口
    sf::RectangleShape centerZoneBorder;       // 中心目标区域边界
    sf::RectangleShape chargeBar;              // 蓄力条指示器
    sf::Image icon;                            // 添加图标成员变量
    // 视觉资源
    TextureManager textureManager;             // 纹理管理器
    sf::Texture backgroundTexture;             // 主游戏背景纹理
    sf::Sprite backgroundSprite;               // 主游戏背景精灵
    sf::Texture backgroundTextureEnd;          // 结束画面背景纹理
    sf::Sprite backgroundSpriteEnd;            // 结束画面背景精灵

    // 文本和界面元素
    sf::Font font;                             // 主游戏字体
    sf::Text scoreText;                        // 当前分数显示
    sf::Text highScoreText;                    // 最高分显示
    sf::Text finalScoreText;                   // 最终分数显示
    sf::Text playerCountText;                  // 剩余发射次数计数器
    sf::Text selectionText;                    // 玩家选择指示器

    // 音频组件
    sf::SoundBuffer collisionBuffer;           // 碰撞音效缓冲
    sf::Sound collisionSound;                  // 碰撞音效播放器
    sf::Music backgroundMusic;                 // 背景音乐

    // 游戏管理器
    ScoreManager scoreManager;                  // 分数管理器

    // 游戏对象容器
    std::vector<GameObject> enemies;            // 敌方球体列表
    std::vector<GameObject> players;            // 玩家球体列表

    // 游戏状态变量
    GameState currentGameState;                 // 当前游戏状态
    bool viewArchiveMode;                       // 存档查看模式标志
    bool allPlayersStopped;                     // 所有玩家停止移动标志

    // 游戏计数器
    int normalCount;                            // 普通球数量
    int specialCount;                           // 特殊球数量
    int hadshoot;                               // 已发射次数
    int selectedPlayerIndex;                    // 当前选中的玩家索引

    // 蓄力系统变量
    bool isCharging;                            // 蓄力状态标志
    float chargeTime;                           // 当前蓄力时间

    
public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"哐哐当当雀雀球"),
             scoreManager("highscores.txt"),
             normalCount(2), specialCount(2), isCharging(false), chargeTime(0.f),
             selectedPlayerIndex(0), hadshoot(0), viewArchiveMode(false), currentGameState(Playing) {
        
        window.setFramerateLimit(60);    // 设置帧率限制

        // 加载并设置窗口图标
       if (icon.loadFromFile("Images/bird_2.png")) {  // 使用你的图标文件
           window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
       }
        // 加载背景图片
        if (!backgroundTexture.loadFromFile("Images/background.png")) {
            std::cerr << "Error: Failed to load background image!" << std::endl;
        } else {
            backgroundSprite.setTexture(backgroundTexture);
        }

        // 加载结束画面背景
        if (!backgroundTextureEnd.loadFromFile("Images/backgroundend.png")) {
            std::cerr << "Error: Failed to load end screen background!" << std::endl;
        } else {
            backgroundSpriteEnd.setTexture(backgroundTextureEnd);
        }

        // 加载字体
        if (!font.loadFromFile("chinese.ttf")) {
            std::cerr << "Error: Could not load font file: chinese.ttf" << std::endl;
            return;
        }

        // 加载并播放背景音乐
        if (!backgroundMusic.openFromFile("background_music.flac")) {
            std::cerr << "Error: Failed to load background music!" << std::endl;
        } else {
            backgroundMusic.setLoop(true);     // 设置循环播放
            backgroundMusic.setVolume(50.f);   // 设置音量 (0.0f - 100.0f)
            backgroundMusic.play();            // 开始播放
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

            switch (currentGameState) {
                case Playing:
                    if (isCharging) updateCharge();
                    updateGameObjects();
                    checkCollisions();
                    updateEnemyCount();
                    updateMessage();

                    allPlayersStopped = std::all_of(players.begin(), players.end(), [](const GameObject& player) {
                        return player.isStopped;
                    });

                    if (hadshoot >= players.size() && allPlayersStopped) {
                        saveGame("savegame.bin");
                        currentGameState = EndScreen;
                    }
                    render();
                    break;
                case EndScreen:
                    renderEndScene();
                    break;
                case ArchiveView:
                    render();
                    break;
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
                    if (dist < ENEMY_RADIUS + ENEMY_RADIUS + 50) {
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

            if (currentGameState == Playing) { // 只在 Playing 状态下处理发射相关事件
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Num1) selectedPlayerIndex = 0;
                    if (event.key.code == sf::Keyboard::Num2) selectedPlayerIndex = 1;
                    if (event.key.code == sf::Keyboard::Num3) selectedPlayerIndex = 2;
                    if (event.key.code == sf::Keyboard::Num4) selectedPlayerIndex = 3;
                }

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && hadshoot < players.size()) {
                    isCharging = true;
                    chargeTime = 0.f;
                }

                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && isCharging) {
                    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
                    launchPlayer(mousePos);
                    isCharging = false;
                    hadshoot++;
                }
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::V) { // V 键事件处理
                if (currentGameState == EndScreen) {
                    loadGame("savegame.bin");
                    currentGameState = ArchiveView;
                } else if (currentGameState == ArchiveView) {
                    currentGameState = EndScreen;
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


    void launchPlayer(const sf::Vector2f& mousePos) {
        if (selectedPlayerIndex >= 0 && selectedPlayerIndex < players.size() && hadshoot < players.size()) {
            auto& player = players[selectedPlayerIndex];
            sf::Vector2f direction = mousePos - player.sprite.getPosition();
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (length > 0) {
                direction /= length;
            }

            player.velocity = direction * (chargeTime / CHARGE_MAX_TIME * 250.f);
            player.isStopped = false;
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

        sf::Text gameOverHighScore;
        sf::Text gameOverCurrentScore;
        sf::Text archiveView;
        sf::Text remainingShots;

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
        archiveView.setString(L"按 V 键查看存档，再次按 V 返回");
        archiveView.setCharacterSize(40);
        archiveView.setFillColor(sf::Color::White);
        archiveView.setPosition(WINDOW_WIDTH / 2 - archiveView.getGlobalBounds().width / 2, WINDOW_HEIGHT - 200);

        window.draw(gameOverHighScore);
        window.draw(gameOverCurrentScore);
        window.draw(archiveView);
        window.draw(remainingShots);

        window.display(); // 只显示一次，不再循环
    }

    void saveGame(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            int currentScore = scoreManager.getCurrentScore(); // 获取当前分数
            file.write(reinterpret_cast<const char*>(&currentScore), sizeof(int));
            file.write(reinterpret_cast<const char*>(&hadshoot), sizeof(int));

            int enemyCount = enemies.size();
            file.write(reinterpret_cast<const char*>(&enemyCount), sizeof(int));
            for (const auto& enemy : enemies) {
                enemy.save(file);
            }

            int playerCount = players.size();
            file.write(reinterpret_cast<const char*>(&playerCount), sizeof(int));
            for (const auto& player : players) {
                player.save(file);
            }

            file.close();
            std::cout << "Game saved successfully!" << std::endl;
        } else {
            std::cerr << "Error saving game!" << std::endl;
        }
    }

    void loadGame(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            int loadedScore;
            file.read(reinterpret_cast<char*>(&loadedScore), sizeof(int));
            scoreManager.updateScore(loadedScore);
            file.read(reinterpret_cast<char*>(&hadshoot), sizeof(int));

            int enemyCount;
            file.read(reinterpret_cast<char*>(&enemyCount), sizeof(int));
            enemies.clear();
            for (int i = 0; i < enemyCount; ++i) {
                enemies.emplace_back(ENEMY_RADIUS, "Images/bird_2.png", sf::Vector2f(0, 0), textureManager);
                enemies.back().load(file);
            }

            int playerCount;
            file.read(reinterpret_cast<char*>(&playerCount), sizeof(int));
            players.clear();
            for (int i = 0; i < playerCount; ++i) {
                players.emplace_back(PLAYER_RADIUS, "Images/bird_1.png", sf::Vector2f(0, 0), textureManager);
                players.back().load(file);
            }

            file.close();
            std::cout << "Game loaded successfully!" << std::endl;
            viewArchiveMode = false;
        } else {
            std::cerr << "Error loading game!" << std::endl;
        }
    }

};