#include <SFML/Graphics.hpp>
#include <iostream>

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



class Button {
public:
    Button(const sf::Texture& texture, const sf::Vector2f& position)
            : m_texture(texture), m_sprite(texture), m_position(position) {}

    void draw(sf::RenderWindow& window) {
        m_sprite.setPosition(m_position);
        m_sprite.

        icon.setScale(scaleX, scaleY);
        icon.setPosition(1359, 927);
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
    sf::
};


int main()
{
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML Background");

    // 加载背景图片
    sf::Texture texture;

    if (!texture.loadFromFile("Images/background_image.png")) {
        std::cerr << "Failed to load background image" << std::endl;
        return -1;
    } else {
        std::cout << "Loaded background image: "
                  << texture.getSize().x << "x" << texture.getSize().y << std::endl;
    }

    // 这是首页显示的代码
    // 创建精灵并设置
    sf::Sprite sprite;
    sprite.setTexture(texture);
    // 调整精灵的大小和位置以适应窗口
    float scaleX = window.getSize().x / static_cast<float>(texture.getSize().x);
    float scaleY = window.getSize().y / static_cast<float>(texture.getSize().y);
    std::cout << "Scale: " << scaleX << ", " << scaleY << std::endl;
    sprite.setScale(scaleX, scaleY);


    //接下来是首页的botton



    sf::Texture buttonTexture;

    if (!buttonTexture.loadFromFile("Images/start_icon.png")) {
        std::cerr << "Failed to load start icon" << std::endl;
        return -1; // 或者其他处理
    } else {
        std::cout << "Loaded start icon: "
                  << buttonTexture.getSize().x << "x" << buttonTexture.getSize().y << std::endl;
    }


    Button button(buttonTexture, sf::Vector2f(100, 100));

    bool isTransitioning = false;
    float transitionTime = 2.0f;
    float currentTime = 0.0f;
    sf::Color transitionColor = sf::Color::White;


    sf::Sprite icon;
    icon.setTexture(buttonTexture);
    icon.setScale(scaleX, scaleY);
    icon.setPosition(1359, 927);



    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (button.isClicked(sf::Mouse::getPosition(window))) {
                    isTransitioning = true;
                    currentTime = 0.0f;
                }
            }







            //测试位置
            if (event.mouseButton.button == sf::Mouse::Right)
            {
                std::cout << "the right button was pressed" << std::endl;
                std::cout << "mouse x: " << event.mouseButton.x << std::endl;
                std::cout << "mouse y: " << event.mouseButton.y << std::endl;
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

        // 清空窗口
        window.clear();

        // 绘制背景
        window.draw(sprite);
        window.draw(icon);
        // 显示窗口
        window.display();
    }

    return 0;
}