#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include "Game.h"

// 动画效果类：实现界面过渡动画
class Easing {
public:
    // 二次方缓动函数：实现平滑的渐入渐出效果
    static float quadraticEaseInOut(float t, float b, float c, float d) {
        t /= d / 2;
        if (t < 1) return c / 2 * t * t + b;
        t--;
        return -c / 2 * (t * (t - 2) - 1) + b;
    }
};

// 按钮类：处理UI按钮的显示和交互
class Button {
private:
    sf::Sprite m_sprite;      // 按钮精灵
    sf::Vector2f m_position;  // 按钮位置
    sf::Vector2f m_scale;     // 按钮缩放

public:
    // 使用默认参数
    Button(const sf::Texture& texture, const sf::Vector2f& position, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f))
            : m_sprite(texture), m_position(position), m_scale(scale) {
        m_sprite.setPosition(m_position);
        m_sprite.setScale(m_scale);
    }

    // 绘制按钮，支持透明度设置
    void draw(sf::RenderTarget& target, float alpha) const {
        sf::Sprite tempSprite = m_sprite;
        tempSprite.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha)));
        target.draw(tempSprite);
    }

    // 检测按钮是否被点击
    bool isClicked(const sf::Vector2i& mousePos) const {
        return m_sprite.getGlobalBounds().contains(static_cast<float>(mousePos.x), 
                                                 static_cast<float>(mousePos.y));
    }
};

// 背景类：处理背景图片的加载和显示
class Background {
private:
    sf::Texture m_texture;  // 背景纹理
    sf::Sprite m_sprite;    // 背景精灵

public:
    // 构造函数：加载并设置背景
    Background(const std::string& filePath, const sf::RenderWindow& window) {
        loadTexture(filePath);
        scaleSprite(window);
    }

    // 加载背景纹理
    void loadTexture(const std::string& filePath) {
        if (!m_texture.loadFromFile(filePath)) {
            std::cerr << "Failed to load image: " << filePath << std::endl;
        }
        m_sprite.setTexture(m_texture);
    }

    // 缩放背景以适应窗口
    void scaleSprite(const sf::RenderWindow& window) {
        float windowAspectRatio = static_cast<float>(window.getSize().x) / window.getSize().y;
        float textureAspectRatio = static_cast<float>(m_texture.getSize().x) / m_texture.getSize().y;

        if (windowAspectRatio > textureAspectRatio) {
            float scaleX = window.getSize().x / static_cast<float>(m_texture.getSize().x);
            m_sprite.setScale(scaleX, scaleX);
        } else {
            float scaleY = window.getSize().y / static_cast<float>(m_texture.getSize().y);
            m_sprite.setScale(scaleY, scaleY);
        }
    }

    // 绘制背景
    void draw(sf::RenderTarget& target) const {
        target.draw(m_sprite);
    }

    // 获取背景缩放比例
    sf::Vector2f getScale() const {
        return m_sprite.getScale();
    }
};

// 应用程序类：管理整个游戏流程
class Application {
private:
    // 窗口和渲染相关
    sf::RenderWindow window;              // 主窗口
    sf::RenderTexture renderTexture;      // 渲染纹理
    float alpha;                          // 透明度值

    // 背景和按钮相关
    Background background;                 // 背景对象
    sf::Texture buttonTexture;            // 开始按钮纹理
    std::unique_ptr<Button> button;       // 开始按钮
    sf::Texture buttonTexture2;           // 第二按钮纹理
    std::unique_ptr<Button> button2;      // 第二按钮
    sf::Image icon;                       // 图标成员变量

    // 过渡动画相关
    bool isTransitioning;                 // 是否正在过渡
    float transitionTime;                 // 过渡时间
    float currentTime;                    // 当前时间
    sf::Clock clock;                      // 时钟
    int currentPage;                      // 当前页面

    // 处理输入事件
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (button && button->isClicked(sf::Mouse::getPosition(window)) && currentPage == 1) {
                    // 先加载新纹理，再重置按钮
                    background.loadTexture("Images/second_page.png");
                    background.scaleSprite(window);
                    startTransition();
                    currentPage = 2;
                    button.reset();  // 移到最后
                } else if (button2 && button2->isClicked(sf::Mouse::getPosition(window)) && currentPage == 2) {
                    // 先加载新纹理，再重置按钮
                    background.loadTexture("Images/background.png");
                    background.scaleSprite(window);
                    startTransition();
                    currentPage = 3;
                    button.reset();  // 移到最后
                    switchToGame();
                }
            }
        }
    }

    // 开始过渡动画
    void startTransition() {
        isTransitioning = true;
        currentTime = 0.0f;
        clock.restart();
    }

    // 更新游戏状态
    void update() {
        if (isTransitioning) {
            currentTime += clock.restart().asSeconds();
            // 检查过渡动画是否完成
            if (currentTime >= transitionTime) {
                isTransitioning = false;
                currentTime = 0.0f;
            }
        }
    }

    // 渲染画面
    void render() {
        if (!renderTexture.create(window.getSize().x, window.getSize().y)) {
            std::cerr << "Failed to create render texture!" << std::endl;
            return;  // 添加提前返回
        }

        renderTexture.clear(sf::Color::Transparent);
        background.draw(renderTexture);
        
        // 检查按钮是否存在再绘制
        if (currentPage == 1 && button) {
            button->draw(renderTexture, 255.0f);
        } else if (currentPage == 2 && button2) {
            button2->draw(renderTexture, isTransitioning ? alpha : 255.0f);
        }

        renderTexture.display();

        // 使用临时精灵绘制渲染纹理
        sf::Sprite renderSprite(renderTexture.getTexture());
        window.clear();  // 添加清除窗口
        window.draw(renderSprite);

        if (isTransitioning) {
            alpha = Easing::quadraticEaseInOut(currentTime, 255.0f, -255.0f, transitionTime);
            sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
            overlay.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha)));
            window.draw(overlay);
        }

        window.display();
    }

    // 切换到游戏场景
    void switchToGame() {
        // 确保在切换前释放资源
        button.reset();
        button2.reset();
        renderTexture.clear();
        
        Game game;
        window.close();
        game.run();
    }

public:
    // 构造函数：初始化应用程序
    Application()
            : window(sf::VideoMode(1920, 1080), L"哐哐当当雀雀球"),
              background("Images/background_image.png", window),
              isTransitioning(false), 
              transitionTime(2.0f), 
              currentTime(0.0f), 
              currentPage(1),
              alpha(255.0f)  // 初始化 alpha
    {
        // 加载并设置窗口图标
       if (icon.loadFromFile("Images/bird_2.png")) {  // 使用你的图标文件
           window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
       }

        // 初始化渲染纹理
        if (!renderTexture.create(window.getSize().x, window.getSize().y)) {
            throw std::runtime_error("Failed to create render texture!");
        }

        // 初始化开始按钮
        if (!buttonTexture.loadFromFile("Images/start_icon.png")) {
            throw std::runtime_error("Failed to load start button icon!");
        }
        button = std::make_unique<Button>(buttonTexture, sf::Vector2f(1359, 927), background.getScale());

        // 初始化第二个按钮
        if (!buttonTexture2.loadFromFile("Images/second_botton.png")) {
            throw std::runtime_error("Failed to load second button!");
        }
        button2 = std::make_unique<Button>(buttonTexture2, sf::Vector2f(1567, 993));
    }

    // 运行应用程序
    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }
};