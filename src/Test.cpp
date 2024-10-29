#include <SFML/Graphics.hpp>
#include <iostream>

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

    sf::Texture iconTexture;

    if (!iconTexture.loadFromFile("Images/start_icon.png")) {
        std::cerr << "Failed to load start icon" << std::endl;
        return -1; // 或者其他处理
    } else {
        std::cout << "Loaded start icon: "
                  << iconTexture.getSize().x << "x" << iconTexture.getSize().y << std::endl;
    }

    sf::Sprite icon;
    icon.setTexture(iconTexture);
    icon.setScale(scaleX, scaleY);
    icon.setPosition(1359, 927);



    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (icon.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        // 鼠标点击在图标上，执行跳转操作
                        // 例如：切换到另一个场景
                        std::cout << "Icon clicked!" << std::endl;
                    }
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