#include <SFML/Graphics.hpp>
#include <cmath> // for math functions

// 定义缓动函数
float linearEase(float t, float b, float c, float d) {
    return c * t / d + b;
}

float quadraticEaseInOut(float t, float b, float c, float d) {
    t /= d / 2;
    if (t < 1) return c / 2 * t * t + b;
    t--;
    return -c / 2 * (t * (t - 2) - 1) + b;
}

// 其他缓动函数...


//
////接下来是首页的botton
//
//sf::Texture iconTexture;
//
//if (!iconTexture.loadFromFile("Images/start_icon.png")) {
//std::cerr << "Failed to load start icon" << std::endl;
//return -1; // 或者其他处理
//} else {
//std::cout << "Loaded start icon: "
//<< iconTexture.getSize().x << "x" << iconTexture.getSize().y << std::endl;
//}
//
//sf::Sprite icon;
//icon.setTexture(iconTexture);
//icon.setScale(scaleX, scaleY);
//icon.setPosition(1359, 927);

class Button {
public:
    Button(const sf::Texture& texture, const sf::Vector2f& position)
            : m_texture(texture), m_sprite(texture), m_position(position) {}

    void draw(sf::RenderWindow& window) {
        m_sprite.setPosition(m_position);
        window.draw(m_sprite);
    }

    bool isClicked(const sf::Vector2i& mousePos) {
        sf::FloatRect bounds = m_sprite.getGlobalBounds();
        return bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    sf::Vector2f m_position;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Button");


    sf::Texture buttonTexture;
    if (!buttonTexture.loadFromFile("your_button_image.png")) {
        // 处理加载失败的情况
    }


    Button button(buttonTexture, sf::Vector2f(100, 100));

    bool isTransitioning = false;
    float transitionTime = 2.0f;
    float currentTime = 0.0f;
    sf::Color transitionColor = sf::Color::White;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (button.isClicked(sf::Mouse::getPosition(window))) {
                    isTransitioning = true;
                    currentTime = 0.0f;
                }
            }
        }

        if (isTransitioning) {
            currentTime += clock.getElapsedTime().asSeconds();
            if (currentTime >= transitionTime) {
                isTransitioning = false;
                currentTime = 0.0f;
            } else {
                // 选择缓动函数
                float alpha = 255.0f * (1.0f - quadraticEaseInOut(currentTime, 0.0f, 1.0f, transitionTime));
                transitionColor.a = static_cast<uint8_t>(alpha);
            }
        }

        window.clear(transitionColor);
        button.draw(window);
        window.display();
    }

    return 0;
}