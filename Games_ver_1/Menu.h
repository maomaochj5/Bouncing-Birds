#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include "Game.h" // 假设你的游戏类名为 Game

// Easing 类
class Easing {
public:
    static float quadraticEaseInOut(float t, float b, float c, float d) {
        t /= d / 2;
        if (t < 1) return c / 2 * t * t + b;
        t--;
        return -c / 2 * (t * (t - 2) - 1) + b;
    }
};

// Button 类
class Button {
public:
    Button(const sf::Texture& texture, const sf::Vector2f& position, const sf::Vector2f& scale)
            : m_sprite(texture), m_position(position), m_scale(scale) {
        m_sprite.setPosition(m_position);
        m_sprite.setScale(m_scale);
    }

    Button(const sf::Texture& texture, const sf::Vector2f& position)
            : m_sprite(texture), m_position(position) {
        m_sprite.setPosition(m_position);
    }

    void draw(sf::RenderTarget& target, float alpha) const {
        sf::Sprite tempSprite = m_sprite;
        tempSprite.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha)));
        target.draw(tempSprite); // 使用 target.draw()
    }

    bool isClicked(const sf::Vector2i& mousePos) const {
        sf::FloatRect bounds = m_sprite.getGlobalBounds();
        return bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

private:
    sf::Sprite m_sprite;
    sf::Vector2f m_position;
    sf::Vector2f m_scale;
};

// Background 类
class Background {
public:
    Background(const std::string& filePath, const sf::RenderWindow& window) {
        loadTexture(filePath);
        scaleSprite(window);
    }

    void loadTexture(const std::string& filePath) {
        if (!m_texture.loadFromFile(filePath)) {
            std::cerr << "Failed to load image: " << filePath << std::endl;
        }
        m_sprite.setTexture(m_texture);
    }

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

    void draw(sf::RenderTarget& target) const {
        target.draw(m_sprite); // 使用 target.draw()
    }

    sf::Vector2f getScale() const {
        return m_sprite.getScale();
    }



private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
};

// Application 类
class Application {
public:
    Application()
            : window(sf::VideoMode(1920, 1080), L"哐哐当当雀雀球"),
              background("Images/background_image.png", window),
              isTransitioning(false), transitionTime(2.0f), currentTime(0.0f), transitionColor(sf::Color::White),
              currentPage(1) {

        if (!buttonTexture.loadFromFile("Images/start_icon.png")) {
            std::cerr << "Failed to load start icon" << std::endl;
        }
        sf::Vector2f buttonPosition(1359, 927);
        button = std::make_unique<Button>(buttonTexture, buttonPosition, background.getScale());

        if (!buttonTexture2.loadFromFile("Images/second_botton.png")) {
            std::cerr << "Failed to load second button" << std::endl;
        }
        sf::Vector2f buttonPosition2(1567, 993);
        button2 = std::make_unique<Button>(buttonTexture2, buttonPosition2);
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }

private:

    float alpha; // 声明 alpha 变量
    sf::RenderTexture renderTexture;

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (button && button->isClicked(sf::Mouse::getPosition(window)) && currentPage == 1) {
                    background.loadTexture("Images/second_page.png");
                    background.scaleSprite(window);
                    isTransitioning = true;
                    currentTime = 0.0f;
                    clock.restart();
                    button.reset();
                    currentPage = 2;
                } else if (button2 && button2->isClicked(sf::Mouse::getPosition(window)) && currentPage == 2) {
                    background.loadTexture("Images/background.png");
                    background.scaleSprite(window);
                    isTransitioning = true;
                    currentTime = 0.0f;
                    clock.restart();
                    button.reset();
                    currentPage = 3;
                    switchToGame(); // 正确的调用位置
                }
            }
        }
    }

    void update() {
        if (isTransitioning) {
            currentTime += clock.restart().asSeconds();
            if (currentTime >= transitionTime) {
                isTransitioning = false;
                currentTime = 0.0f;
            }
        }
    }

    void drawButton(Button* button, sf::RenderWindow& window, float alpha) {
        if (button) {
            button->draw(window, alpha);
        }
    }

    void render() {
        // 确保 renderTexture 已创建且大小正确
        if (renderTexture.create(window.getSize().x, window.getSize().y)) {
            renderTexture.clear(sf::Color::Transparent); // 清除纹理，非常重要！

            // 绘制背景到 renderTexture
            background.draw(renderTexture);

            // 根据 currentPage 绘制按钮到 renderTexture，并应用透明度
            if (currentPage == 1 && button) {
                button->draw(renderTexture, 255.0f); // 第一页按钮不透明
            } else if (currentPage == 2 && button2) {
                button2->draw(renderTexture, isTransitioning ? alpha : 255.0f); // 根据过渡状态应用透明度
            }

            renderTexture.display();

            // 将 renderTexture 绘制到窗口
            sf::Sprite renderSprite(renderTexture.getTexture());
            window.draw(renderSprite);

            // 如果正在过渡，绘制覆盖层（现在 alpha 计算放在这里）
            if (isTransitioning) {
                alpha = Easing::quadraticEaseInOut(currentTime, 255.0f, -255.0f, transitionTime);
                sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
                overlay.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha)));
                window.draw(overlay);
            }
        } else {
            std::cerr << "Failed to create render texture!" << std::endl;
        }

        window.display();
    }

    void switchToGame() {
        Game game;
        window.close();
        game.run();
    }

    sf::RenderWindow window;
    Background background;
    sf::Texture buttonTexture;
    std::unique_ptr<Button> button;
    sf::Texture buttonTexture2;
    std::unique_ptr<Button> button2;

    bool isTransitioning;
    float transitionTime;
    float currentTime;
    sf::Color transitionColor;
    sf::Clock clock;

    int currentPage;
};