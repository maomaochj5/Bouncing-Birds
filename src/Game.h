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
#include <functional>

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

// Add this with other constants at the top
constexpr const char* FINAL_SAVE_FILE = "final_save.bin";

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

    // 旋转相关属性
    float angularVelocity;              // 角速度
    float rotationDamping;              // 旋转阻尼系数

    // 特殊球属性
    bool isSpecial;                     // 是否为特殊球
    bool hasTriggeredSpecial;           // 是否已触发特殊效果
    bool hasBeenLaunched;               // 是否已被发射

    // 特殊效果回调函数
    std::function<void(GameObject&)> onSpecialEffect;

    // 构造函数：初始化游戏对象
    GameObject(float radius, const std::string& textureFile, sf::Vector2f position, 
              TextureManager& textureManager, float m = 1.0f)
            : velocity(0.f, 0.f), mass(m), isStopped(true), 
              angularVelocity(0.f), rotationDamping(0.98f),
              isSpecial(false), hasTriggeredSpecial(false),
              hasBeenLaunched(false) {
        sprite.setTexture(textureManager.getTexture(textureFile));
        
        // 确保将原点设置在纹理的中心
        sf::Vector2u textureSize = sprite.getTexture()->getSize();
        sprite.setOrigin(textureSize.x / 2.f, textureSize.y / 2.f);
        
        // 设置位置和缩放
        sprite.setPosition(position);
        float scaleX = (radius * 2) / static_cast<float>(textureSize.x);
        float scaleY = (radius * 2) / static_cast<float>(textureSize.y);
        sprite.setScale(scaleX, scaleY);
    }

    // 渲染方法
    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    // 更新物体位置和状态
    void updatePosition(float dt) {
        if (!isStopped) {
            hasBeenLaunched = true;
            sprite.move(velocity * dt);
            velocity *= FRICTION_COEFFICIENT;

            // 更新旋转
            float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            angularVelocity = -speed * 0.5f;
            sprite.rotate(angularVelocity * dt);
            
            // 检查停止条件
            if (std::abs(velocity.x) < 0.01f && std::abs(velocity.y) < 0.01f) {
                isStopped = true;
                velocity = {0.f, 0.f};
                angularVelocity = 0.f;

                // 触发特殊效果（如果是特殊球）
                if (isSpecial && hasBeenLaunched && !hasTriggeredSpecial && onSpecialEffect) {
                    onSpecialEffect(*this);
                    hasTriggeredSpecial = true;
                }
            }
        }
    }

    // 边界碰撞检测和处理
    void applyBoundaryCollision() {
        auto bounds = sprite.getGlobalBounds();

        // 检测左右边界碰撞
        if (bounds.left < 280 || bounds.left + bounds.width > WINDOW_WIDTH - 300) {
            velocity.x = -velocity.x * REBOUND_COEFFICIENT;
            sprite.setPosition(
                std::max(bounds.width / 2, 
                std::min(sprite.getPosition().x, WINDOW_WIDTH - bounds.width / 2)), 
                sprite.getPosition().y
            );
        }

        // 检测上下边界碰撞
        if (bounds.top < 120 || bounds.top + bounds.height > WINDOW_HEIGHT) {
            velocity.y = -velocity.y * REBOUND_COEFFICIENT;
            sprite.setPosition(
                sprite.getPosition().x, 
                std::max(bounds.height / 2, 
                std::min(sprite.getPosition().y, WINDOW_HEIGHT - bounds.height / 2))
            );
        }
    }

    // 保存对象状态
    void save(std::ofstream& file) const {
        std::vector<char> buffer(sizeof(float) * 6 + sizeof(bool) * 4);
        char* ptr = buffer.data();

        sf::Vector2f pos = sprite.getPosition();
        float rotation = sprite.getRotation();

        // 保存位置、速度和旋转信息
        std::memcpy(ptr, &pos.x, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &pos.y, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &velocity.x, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &velocity.y, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &angularVelocity, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(ptr, &rotation, sizeof(float));
        ptr += sizeof(float);

        // 保存状态标志
        std::memcpy(ptr, &isStopped, sizeof(bool));
        ptr += sizeof(bool);
        std::memcpy(ptr, &isSpecial, sizeof(bool));
        ptr += sizeof(bool);
        std::memcpy(ptr, &hasTriggeredSpecial, sizeof(bool));
        ptr += sizeof(bool);
        std::memcpy(ptr, &hasBeenLaunched, sizeof(bool));

        file.write(buffer.data(), buffer.size());
    }

    // 加载对象状态
    void load(std::ifstream& file) {
        std::vector<char> buffer(sizeof(float) * 6 + sizeof(bool) * 4);
        file.read(buffer.data(), buffer.size());
        char* ptr = buffer.data();

        sf::Vector2f pos;
        float rotation;

        // 加载位置、速度和旋转信息
        std::memcpy(&pos.x, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&pos.y, ptr, sizeof(float));
        ptr += sizeof(float);
        sprite.setPosition(pos);

        std::memcpy(&velocity.x, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&velocity.y, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&angularVelocity, ptr, sizeof(float));
        ptr += sizeof(float);
        std::memcpy(&rotation, ptr, sizeof(float));
        ptr += sizeof(float);
        sprite.setRotation(rotation);
        
        // 加载状态标志
        std::memcpy(&isStopped, ptr, sizeof(bool));
        ptr += sizeof(bool);
        std::memcpy(&isSpecial, ptr, sizeof(bool));
        ptr += sizeof(bool);
        std::memcpy(&hasTriggeredSpecial, ptr, sizeof(bool));
        ptr += sizeof(bool);
        std::memcpy(&hasBeenLaunched, ptr, sizeof(bool));
    }
};

// 碰撞处理器：处理游戏对象之间的碰撞
class CollisionHandler {
public:
    static void applyCollision(GameObject& a, GameObject& b, sf::Sound& collisionSound) {
        // 获取两个球的圆心位置
        sf::Vector2f posA = a.sprite.getPosition();
        sf::Vector2f posB = b.sprite.getPosition();

        // 计算两球中心之间的距离
        sf::Vector2f delta = posA - posB;
        float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        
        // 获取两球的半径
        float radiusA = a.sprite.getGlobalBounds().width / 2;
        float radiusB = b.sprite.getGlobalBounds().width / 2;

        // 检查是否发生碰撞（两球中心距离是否小于半径之和）
        if (distance < radiusA + radiusB) {
            collisionSound.play();  // 播放碰撞音效

            // 计算碰撞法线（单位向量）
            sf::Vector2f normal = delta / distance;

            // 计算相对速度
            auto relativeVelocity = a.velocity - b.velocity;
            float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

            // 如果物体正在分离，则不处理碰撞
            if (velocityAlongNormal > 0) return;

            // 更新速度（基于动量守恒和能量守恒）
            sf::Vector2f temp = a.velocity;
            a.velocity = 0.5f * (a.velocity + b.velocity + REBOUND_COEFFICIENT * (b.velocity - a.velocity));
            b.velocity = 0.5f * (b.velocity + temp + REBOUND_COEFFICIENT * (temp - b.velocity));

            // 计算碰撞后的速度大小
            float speedA = std::sqrt(a.velocity.x * a.velocity.x + a.velocity.y * a.velocity.y);
            float speedB = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.y * b.velocity.y);
            
            // 根据碰撞点的相对位置决定旋转方向
            float crossProduct = normal.x * relativeVelocity.y - normal.y * relativeVelocity.x;
            float rotationFactor = 0.5f;
            
            // 更新角速度
            a.angularVelocity = -speedA * rotationFactor * (crossProduct > 0 ? 1 : -1);
            b.angularVelocity = -speedB * rotationFactor * (crossProduct > 0 ? -1 : 1);

            // 防止球体重叠
            float overlap = (radiusA + radiusB - distance) / 2.0f;
            sf::Vector2f separation = normal * overlap;
            a.sprite.setPosition(posA + separation);
            b.sprite.setPosition(posB - separation);

            // 设置运动状态
            a.isStopped = false;
            b.isStopped = false;
        }
    }
};

// 分数管理器：处理游戏分数的记录和保存
class ScoreManager {
private:
    int currentScore;     // 当前分数
    int highScore;        // 最高分数
    std::string filename; // 分数文件名

public:
    // 构造函数：初始化分数管理器
    ScoreManager(const std::string& fname) : currentScore(0), filename(fname) {
        loadHighScore();
    }

    // 更新当前分数
    void updateScore(int score) {
        currentScore = score;
        if (currentScore > highScore) {
            highScore = currentScore;
        }
    }

    // 获取当前分数
    int getCurrentScore() const {
        return currentScore;
    }

    // 获取最高分数
    int getHighScore() const {
        return highScore;
    }

    // 保存最高分数到文件
    void saveScore() {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }

private:
    // 从文件加载最高分数
    void loadHighScore() {
        std::ifstream file(filename);
        if (file.is_open()) {
            file >> highScore;
            file.close();
        } else {
            highScore = 0;
        }
    }
};

// 游戏状态枚举：定义游戏的不同状态
enum GameState {
    Playing,                            // 游戏进行中
    EndScreen,                          // 结束画面
    ArchiveView                         // 存档查看
};

// 游戏主类：管理整个游戏的运行
class Game {
private:
    // 窗口和图形相关
    sf::RenderWindow window;            // 游戏窗口
    sf::Image icon;                     // 窗口图标
    sf::Font font;                      // 游戏字体
    TextureManager textureManager;      // 纹理管理器

    // 背景相关
    sf::Texture backgroundTexture;      // 游戏背景纹理
    sf::Sprite backgroundSprite;        // 游戏背景精灵
    sf::Texture backgroundTextureEnd;   // 结束画面背景纹理
    sf::Sprite backgroundSpriteEnd;     // 结束画面背景精灵

    // 音频相关
    sf::Music backgroundMusic;          // 背景音乐
    sf::SoundBuffer collisionBuffer;    // 碰撞音效缓冲
    sf::Sound collisionSound;          // 碰撞音效

    // 游戏对象
    std::vector<GameObject> enemies;    // 敌方球体
    std::vector<GameObject> players;    // 玩家球体

    // UI元素
    sf::Text scoreText;                // 分数显示
    sf::Text highScoreText;            // 最高分显示
    sf::Text playerCountText;          // 玩家数量显示
    sf::Text selectionText;            // 选择提示文本
    sf::RectangleShape centerZoneBorder; // 中心区域边界
    sf::RectangleShape chargeBar;      // 蓄力条

    // 游戏状态管理
    ScoreManager scoreManager;          // 分数管理器
    GameState currentGameState;         // 当前游戏状态
    bool viewArchiveMode;              // 存档查看模式标志
    bool allPlayersStopped;            // 所有玩家停止标志

    // 游戏计数器
    int normalCount;                   // 普通球数量
    int specialCount;                  // 特殊球数量
    int hadshoot;                      // 已发射次数
    int selectedPlayerIndex;           // 当前选中的玩家索引
    int archiveShootCount;            // 存档查看模式下的额外击球次数

    // 蓄力系统
    bool isCharging;                   // 蓄力状态标志
    float chargeTime;                  // 当前蓄力时间

public:
    // 构造函数：初始化游戏
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"哐哐当当雀雀球"),
             scoreManager("highscores.txt"),
             normalCount(2), specialCount(2), isCharging(false), chargeTime(0.f),
             selectedPlayerIndex(0), hadshoot(0), viewArchiveMode(false), 
             currentGameState(Playing), archiveShootCount(0) {
        
        // 设置窗口属性
        window.setFramerateLimit(60);

        // 加载窗口图标
        if (icon.loadFromFile("Images/bird_2.png")) {
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

        // 加载并设置背景音乐
        if (!backgroundMusic.openFromFile("background_music.flac")) {
            std::cerr << "Error: Failed to load background music!" << std::endl;
        } else {
            backgroundMusic.setLoop(true);
            backgroundMusic.setVolume(50.f);
            backgroundMusic.play();
        }

        // 初始化UI文本
        initializeText(scoreText, 30, sf::Vector2f(WINDOW_WIDTH - 600, 140));
        initializeText(highScoreText, 30, sf::Vector2f(WINDOW_WIDTH - 600, 185));
        initializeText(playerCountText, 30, sf::Vector2f(WINDOW_WIDTH - 550, WINDOW_HEIGHT - 220));
        initializeText(selectionText, 24, sf::Vector2f(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT - 150));

        // 设置中心区域边界
        centerZoneBorder.setSize(CENTER_ZONE_SIZE);
        centerZoneBorder.setOutlineThickness(30);
        centerZoneBorder.setOutlineColor(sf::Color::Green);
        centerZoneBorder.setFillColor(sf::Color::Transparent);
        centerZoneBorder.setPosition(CENTER_ZONE_POSITION);

        // 设置蓄力条
        chargeBar.setSize(sf::Vector2f(30, 200));
        chargeBar.setFillColor(sf::Color::Yellow);
        chargeBar.setPosition(WINDOW_WIDTH - 180, WINDOW_HEIGHT - 620);

        // 加载碰撞音效
        collisionBuffer.loadFromFile("collision.flac");
        collisionSound.setBuffer(collisionBuffer);

        // 初始化游戏对象
        initializeEnemies();
        initializePlayers();
    }

    // 游戏主循环
    void run() {
        while (window.isOpen()) {
            handleEvents();
            bool hasSpecialBallPending = false;

            switch (currentGameState) {
                case Playing:
                    if (isCharging) updateCharge();
                    updateGameObjects();
                    checkCollisions();
                    updateEnemyCount();
                    updateMessage();
                    
                    // 首先检查所有球是否停止
                    allPlayersStopped = std::all_of(players.begin(), players.end(), 
                        [](const GameObject& player) { return player.isStopped; });

                    // 检查是否有特殊球需要触发效果
                    if (allPlayersStopped && hadshoot >= players.size()) {
                        for (const auto& player : players) {
                            if (player.isSpecial && player.hasBeenLaunched && !player.hasTriggeredSpecial) {
                                hasSpecialBallPending = true;
                                break;
                            }
                        }
                    }

                    // 只有在以下条件全部满足时才结束游戏：
                    // 1. 所有球都已发射
                    // 2. 所有球都已停止
                    // 3. 没有待触发的特殊效果
                    // 4. 所有特殊球的效果都已触发完成
                    if (hadshoot >= players.size() && allPlayersStopped && !hasSpecialBallPending) {
                        bool allEffectsCompleted = std::all_of(players.begin(), players.end(),
                            [](const GameObject& player) {
                                return !player.isSpecial || !player.hasBeenLaunched || player.hasTriggeredSpecial;
                            });
                        
                        // 检查所有球（包括敌人）是否都已停止
                        bool allBallsStopped = std::all_of(enemies.begin(), enemies.end(),
                            [](const GameObject& enemy) { return enemy.isStopped; });
                        
                        if (allEffectsCompleted && allBallsStopped) {
                            saveGame(FINAL_SAVE_FILE);  
                            currentGameState = EndScreen;
                        }
                    }
                    render();
                    break;

                case EndScreen:
                    renderEndScene();
                    break;

                case ArchiveView:
                    if (isCharging) updateCharge();
                    updateGameObjects();
                    checkCollisions();
                    updateEnemyCount();
                    updateMessage();
                    render();
                    break;
            }
        }
        scoreManager.saveScore();
    }

private:
    // 初始化文本对象
    void initializeText(sf::Text &text, int size, sf::Vector2f position) {
        text.setFont(font);
        text.setCharacterSize(size);
        text.setPosition(position);
        text.setFillColor(sf::Color::White);
    }

    // 初始化敌方球体
    void initializeEnemies() {
        for (int i = 0; i < NUM_ENEMIES; ++i) {
            sf::Vector2f position;
            bool validPosition = false;

            // 寻找有效的初始位置
            while (!validPosition) {
                float x = CENTER_ZONE_POSITION.x + std::rand() % static_cast<int>(CENTER_ZONE_SIZE.x);
                float y = CENTER_ZONE_POSITION.y + std::rand() % static_cast<int>(CENTER_ZONE_SIZE.y);
                position = sf::Vector2f(x, y);
                validPosition = true;

                // 检查是否在中心区域内
                if (position.x - ENEMY_RADIUS < CENTER_ZONE_POSITION.x ||
                    position.x + ENEMY_RADIUS > CENTER_ZONE_POSITION.x + CENTER_ZONE_SIZE.x ||
                    position.y - ENEMY_RADIUS < CENTER_ZONE_POSITION.y ||
                    position.y + ENEMY_RADIUS > CENTER_ZONE_POSITION.y + CENTER_ZONE_SIZE.y) {
                    validPosition = false;
                    continue;
                }

                // 检查与其他敌方球体的距离
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

    // 初始化玩家球体
    void initializePlayers() {
        selectedPlayerIndex = 0;
        players = {
            GameObject(PLAYER_RADIUS, "Images/bird_1.png",
                      sf::Vector2f(WINDOW_WIDTH / 2 - 170, WINDOW_HEIGHT - 170), textureManager),
            GameObject(PLAYER_RADIUS, "Images/bird_1.png", 
                      sf::Vector2f(WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT - 170), textureManager),
            GameObject(PLAYER_RADIUS, "Images/bird_1.png", 
                      sf::Vector2f(WINDOW_WIDTH / 2 + 30, WINDOW_HEIGHT - 170), textureManager, 1.0f),
            GameObject(PLAYER_RADIUS, "Images/bird_1.png",
                      sf::Vector2f(WINDOW_WIDTH / 2 + 130, WINDOW_HEIGHT - 170), textureManager, 1.0f)
        };
        
        // 设置第3和第4个球为特殊球，并设置它们的特殊效果回调
        players[2].isSpecial = true;
        players[3].isSpecial = true;
        
        // 为特殊球设置回调函数
        auto specialEffect = [this](GameObject& ball) {
            this->triggerSpecialEffect(ball);
        };
        
        players[2].onSpecialEffect = specialEffect;
        players[3].onSpecialEffect = specialEffect;

        // 确保所有球的初始状态正确
        for (auto& player : players) {
            player.isStopped = true;
            player.hasBeenLaunched = false;
            player.hasTriggeredSpecial = false;
            player.velocity = sf::Vector2f(0.f, 0.f);
            player.angularVelocity = 0.f;
        }
    }

    // 特殊效果触发函数
    void triggerSpecialEffect(GameObject& specialBall) {
        const float EFFECT_RADIUS = 150.f;
        const float PUSH_FORCE = 100.f;

        // 对敌方球施加效果
        for (auto& enemy : enemies) {
            sf::Vector2f delta = enemy.sprite.getPosition() - specialBall.sprite.getPosition();
            float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);

            if (distance < EFFECT_RADIUS && distance > 0) {
                sf::Vector2f direction = delta / distance;
                float forceMagnitude = PUSH_FORCE * (1.0f - distance / EFFECT_RADIUS);
                enemy.velocity += direction * forceMagnitude;
                enemy.isStopped = false;
            }
        }

        // 对其他玩家球施加效果
        for (auto& player : players) {
            if (&player != &specialBall) {
                sf::Vector2f delta = player.sprite.getPosition() - specialBall.sprite.getPosition();
                float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);

                if (distance < EFFECT_RADIUS && distance > 0) {
                    sf::Vector2f direction = delta / distance;
                    float forceMagnitude = PUSH_FORCE * (1.0f - distance / EFFECT_RADIUS);
                    player.velocity += direction * forceMagnitude;
                    player.isStopped = false;
                }
            }
        }
    }

    // 事件处理
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (currentGameState == Playing) {  // 正常游戏模式
                handlePlayingStateEvents(event);
            } 
            else if (currentGameState == ArchiveView) {  // 存档查看模式
                handleArchiveViewEvents(event);
            }

            // V键切换游戏状态
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::V) {
                handleStateTransition();
            }
        }
    }

    // 处理正常游戏模式的事件
    void handlePlayingStateEvents(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            handleKeyPressedEvents(event);
        }

        // 正常游戏模式下保持发射次数限制
        if (hadshoot < players.size()) {
            handleMouseEvents(event);
        }
    }

    // 处理存档查看模式的事件
    void handleArchiveViewEvents(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Num1) selectedPlayerIndex = 0;
            if (event.key.code == sf::Keyboard::Num2) selectedPlayerIndex = 1;
            if (event.key.code == sf::Keyboard::Num3) selectedPlayerIndex = 2;
            if (event.key.code == sf::Keyboard::Num4) selectedPlayerIndex = 3;
        }

        // 存档查看模式下无发射次数限制
        handleMouseEvents(event);
        if (event.type == sf::Event::MouseButtonReleased && 
            event.mouseButton.button == sf::Mouse::Left && 
            isCharging) {
            archiveShootCount++;  // 增加额外击球计数
        }
    }

    // 处理按键事件
    void handleKeyPressedEvents(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::R) {
            loadGame("savegame.bin");
        }
        if (event.key.code == sf::Keyboard::S) {
            saveGame("savegame.bin");
        }
        if (event.key.code == sf::Keyboard::Num1) selectedPlayerIndex = 0;
        if (event.key.code == sf::Keyboard::Num2) selectedPlayerIndex = 1;
        if (event.key.code == sf::Keyboard::Num3) selectedPlayerIndex = 2;
        if (event.key.code == sf::Keyboard::Num4) selectedPlayerIndex = 3;
    }

    // 处理鼠标事件
    void handleMouseEvents(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed && 
            event.mouseButton.button == sf::Mouse::Left) {
            isCharging = true;
            chargeTime = 0.f;
        }

        if (event.type == sf::Event::MouseButtonReleased && 
            event.mouseButton.button == sf::Mouse::Left && 
            isCharging) {
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
            launchPlayer(mousePos);
            isCharging = false;
        }
    }

    // 处理游戏状态转换
    void handleStateTransition() {
        if (currentGameState == EndScreen) {
            loadGame(FINAL_SAVE_FILE);  
            currentGameState = ArchiveView;
            archiveShootCount = 0;  // 重置额外击球计数
        } else if (currentGameState == ArchiveView) {
            currentGameState = EndScreen;
        }
    }

    // 更新蓄力状态
    void updateCharge() {
        if (isCharging) {
            if (currentGameState == Playing && hadshoot >= players.size()) {
                // 在正常游戏模式下且已达到发射限制时，不更新蓄力
                return;
            }
            
            chargeTime += 0.016f;
            if (chargeTime > CHARGE_MAX_TIME) {
                chargeTime = CHARGE_MAX_TIME;
            }
            chargeBar.setSize(sf::Vector2f(30, chargeTime / CHARGE_MAX_TIME * 200));
        }
    }

    // 发射玩家球体
    void launchPlayer(const sf::Vector2f& mousePos) {
        if (currentGameState == Playing && hadshoot >= players.size()) {
            // 在正常游戏模式下检查发射限制
            return;
        }

        if (selectedPlayerIndex >= 0 && selectedPlayerIndex < players.size()) {
            auto& player = players[selectedPlayerIndex];
            sf::Vector2f direction = mousePos - player.sprite.getPosition();
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (length > 0) {
                direction /= length;
            }

            player.velocity = direction * (chargeTime / CHARGE_MAX_TIME * 250.f);
            player.isStopped = false;

            // 在这里增加计数，而不是在事件处理中
            if (currentGameState == Playing) {
                hadshoot++;
            } else if (currentGameState == ArchiveView) {
                archiveShootCount++;
            }
        }
    }

    // 更新游戏对象状态
    void updateGameObjects() {
        for (auto& enemy : enemies) {
            enemy.updatePosition(0.1f);
        }
        for (auto& player : players) {
            player.updatePosition(0.1f);
        }
    }

    // 更新敌人计数和分数
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
        scoreText.setString(L"本局分数： " + std::to_wstring(scoreManager.getCurrentScore()));
    }

    // 更新游戏信息显示
    void updateMessage() {
        highScoreText.setString(L"历史记录： " + std::to_wstring(scoreManager.getHighScore()));
        
        // 根据游戏状态显示不同的信息
        if (currentGameState == Playing) {
            playerCountText.setString(L"剩余次数： " + std::to_wstring(std::max(0, (int)players.size() - hadshoot)));
        } else if (currentGameState == ArchiveView) {
            playerCountText.setString(L"额外击球： " + std::to_wstring(archiveShootCount));
        }
    }

    // 检查碰撞
    void checkCollisions() {
        // 应用边界碰撞
        for (auto &player: players) player.applyBoundaryCollision();
        for (auto &enemy: enemies) enemy.applyBoundaryCollision();

        // 检测玩家与敌人之间的碰撞
        for (size_t i = 0; i < players.size(); ++i) {
            for (size_t j = 0; j < enemies.size(); ++j) {
                CollisionHandler::applyCollision(players[i], enemies[j], collisionSound);
            }
        }

        // 检测玩家之间的碰撞
        for (size_t i = 0; i < players.size(); ++i) {
            for (size_t j = i + 1; j < players.size(); ++j) {
                CollisionHandler::applyCollision(players[i], players[j], collisionSound);
            }
        }

        // 检测敌人之间的碰撞
        for (size_t i = 0; i < enemies.size(); ++i) {
            for (size_t j = i + 1; j < enemies.size(); ++j) {
                CollisionHandler::applyCollision(enemies[i], enemies[j], collisionSound);
            }
        }
    }

    // 渲染游戏场景
    void render() {
        window.clear();
        window.draw(backgroundSprite);
        window.draw(centerZoneBorder);
        window.draw(scoreText);
        window.draw(highScoreText);
        window.draw(playerCountText);
        window.draw(chargeBar);

        for (const auto &enemy: enemies) enemy.draw(window);
        for (const auto &player: players) player.draw(window);

        window.draw(selectionText);
        window.display();
    }

    // 渲染结束场景
    void renderEndScene() {
        window.clear();
        window.draw(backgroundSpriteEnd);

        sf::Text gameOverHighScore;
        sf::Text gameOverCurrentScore;
        sf::Text archiveView;
        sf::Text remainingShots;

        // 设置结束画面文本
        gameOverHighScore.setFont(font);
        gameOverHighScore.setString(L"历史记录： " + std::to_wstring(scoreManager.getHighScore()));
        gameOverHighScore.setCharacterSize(70);
        gameOverHighScore.setFillColor(sf::Color::White);
        gameOverHighScore.setPosition(WINDOW_WIDTH / 2 - gameOverHighScore.getGlobalBounds().width / 2, 
                                    WINDOW_HEIGHT / 2 - gameOverHighScore.getGlobalBounds().height / 2 + 40);

        gameOverCurrentScore.setFont(font);
        gameOverCurrentScore.setString(L"本局分数： " + std::to_wstring(scoreManager.getCurrentScore()));
        gameOverCurrentScore.setCharacterSize(70);
        gameOverCurrentScore.setFillColor(sf::Color::White);
        gameOverCurrentScore.setPosition(WINDOW_WIDTH / 2 - gameOverCurrentScore.getGlobalBounds().width / 2, 
                                       WINDOW_HEIGHT / 2 - gameOverCurrentScore.getGlobalBounds().height / 2 - 100);

        archiveView.setFont(font);
        archiveView.setString(L"按 V 键查看存档，再次按 V 返回");
        archiveView.setCharacterSize(40);
        archiveView.setFillColor(sf::Color::White);
        archiveView.setPosition(WINDOW_WIDTH / 2 - archiveView.getGlobalBounds().width / 2, WINDOW_HEIGHT - 200);

        // 绘制结束画面元素
        window.draw(gameOverHighScore);
        window.draw(gameOverCurrentScore);
        window.draw(archiveView);
        window.draw(remainingShots);
        window.display();
    }

    void saveGame(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            // 保存当前分数
            int currentScore = scoreManager.getCurrentScore();
            file.write(reinterpret_cast<const char*>(&currentScore), sizeof(int));
            
            // 保存已发射次数
            file.write(reinterpret_cast<const char*>(&hadshoot), sizeof(int));

            // 保存额外击球次数
            file.write(reinterpret_cast<const char*>(&archiveShootCount), sizeof(int));

            // 保存敌方球体状态
            int enemyCount = enemies.size();
            file.write(reinterpret_cast<const char*>(&enemyCount), sizeof(int));
            for (const auto& enemy : enemies) {
                enemy.save(file);
            }

            // 保存玩家球体状态
            int playerCount = players.size();
            file.write(reinterpret_cast<const char*>(&playerCount), sizeof(int));
            for (const auto& player : players) {
                player.save(file);
            }

            file.close();
        } else {
            std::cerr << "Error saving game!" << std::endl;
        }
    }

    void loadGame(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            // 加载当前分数
            int loadedScore;
            file.read(reinterpret_cast<char*>(&loadedScore), sizeof(int));
            scoreManager.updateScore(loadedScore);
            
            // 加载已发射次数
            file.read(reinterpret_cast<char*>(&hadshoot), sizeof(int));

            // 加载额外击球次数
            file.read(reinterpret_cast<char*>(&archiveShootCount), sizeof(int));

            // 加载敌方球体状态
            int enemyCount;
            file.read(reinterpret_cast<char*>(&enemyCount), sizeof(int));
            enemies.clear();
            for (int i = 0; i < enemyCount; ++i) {
                enemies.emplace_back(ENEMY_RADIUS, "Images/bird_2.png", sf::Vector2f(0, 0), textureManager);
                enemies.back().load(file);
            }

            // 加载玩家球体状态
            int playerCount;
            file.read(reinterpret_cast<char*>(&playerCount), sizeof(int));
            players.clear();
            for (int i = 0; i < playerCount; ++i) {
                players.emplace_back(PLAYER_RADIUS, "Images/bird_1.png", sf::Vector2f(0, 0), textureManager);
                players.back().load(file);
            }

            // 重新设置特殊球的回调函数和状态
            auto specialEffect = [this](GameObject& ball) {
                this->triggerSpecialEffect(ball);
            };
            
            // 为第3和第4个球重新设置特殊效果
            if (players.size() > 2) {
                players[2].isSpecial = true;
                players[2].onSpecialEffect = specialEffect;
                // 如果球已经被发射但还没触发效果，重置其触发状态
                if (players[2].hasBeenLaunched && players[2].hasTriggeredSpecial) {
                    players[2].hasTriggeredSpecial = false;
                }
            }
            if (players.size() > 3) {
                players[3].isSpecial = true;
                players[3].onSpecialEffect = specialEffect;
                // 如果球已经被发射但还没触发效果，重置其触发状态
                if (players[3].hasBeenLaunched && players[3].hasTriggeredSpecial) {
                    players[3].hasTriggeredSpecial = false;
                }
            }

            file.close();
            viewArchiveMode = false;
            currentGameState = Playing;  // 确保状态被重置为Playing
        } else {
            std::cerr << "Error loading game!" << std::endl;
        }
    }
};
