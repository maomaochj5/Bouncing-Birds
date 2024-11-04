#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

class Easing {
public:
    static float linearEase(float t, float b, float c, float d) {
        return c * t / d + b;
    }

    static float quadraticEaseInOut(float t, float b, float c, float d) {
        t /= d / 2;
        if (t < 1) return c / 2 * t * t + b;
        t--;
        return -c / 2 * (t * (t - 2) - 1) + b;
    }
};

class Button {
public:
    Button(const sf::Texture& texture, const sf::Vector2f& position, const sf::Vector2f& scale)
            : m_sprite(texture), m_position(position), m_scale(scale) {
        m_sprite.setPosition(m_position);
        m_sprite.setScale(m_scale);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(m_sprite);
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
            // 窗口更宽，按宽度缩放
            float scaleX = window.getSize().x / static_cast<float>(m_texture.getSize().x);
            m_sprite.setScale(scaleX, scaleX);
        } else {
            // 窗口更高，按高度缩放
            float scaleY = window.getSize().y / static_cast<float>(m_texture.getSize().y);
            m_sprite.setScale(scaleY, scaleY);
        }
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(m_sprite);
    }

    sf::Vector2f getScale() const {
        return m_sprite.getScale();
    }

private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
};

class Application {
public:
    Application()
            : window(sf::VideoMode(1920, 1080), "SFML Background"),
              background("Images/background_image.png", window),
              isTransitioning(false), transitionTime(2.0f), currentTime(0.0f), transitionColor(sf::Color::White) {

        if (buttonTexture.loadFromFile("Images/start_icon.png")) {
            sf::Vector2f buttonPosition(1359, 927);
            button = std::make_unique<Button>(buttonTexture, buttonPosition, background.getScale());
        } else {
            std::cerr << "Failed to load start icon" << std::endl;
        }
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }

private:
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (button && button->isClicked(sf::Mouse::getPosition(window))) {
                    // 切换到第二页面的背景
                    background.loadTexture("Images/second_page.png");
                    background.scaleSprite(window); // 确保背景按窗口大小缩放
                    isTransitioning = true;
                    currentTime = 0.0f;
                    clock.restart();
                    button.reset(); // 移除按钮
                }
            }

            if (event.mouseButton.button == sf::Mouse::Right)
            {
                std::cout << "the right button was pressed" << std::endl;
                std::cout << "mouse x: " << event.mouseButton.x << std::endl;
                std::cout << "mouse y: " << event.mouseButton.y << std::endl;
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

    void render() {
        window.clear();

        // 绘制背景
        background.draw(window);

        // 如果正在过渡，使用渐变颜色
        if (isTransitioning) {
            sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
            overlay.setFillColor(transitionColor);
            float alpha = 255.0f * (1.0f - Easing::quadraticEaseInOut(currentTime, 0.0f, 1.0f, transitionTime));
            overlay.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha)));
            window.draw(overlay); // 绘制覆盖层
        }

        // 绘制按钮
        if (button) button->draw(window);

        window.display();
    }

    sf::RenderWindow window;
    Background background;
    sf::Texture buttonTexture;
    std::unique_ptr<Button> button;

    bool isTransitioning;
    float transitionTime;
    float currentTime;
    sf::Color transitionColor;
    sf::Clock clock;
};

int main() {
    Application app;
    app.run();
    return 0;
}