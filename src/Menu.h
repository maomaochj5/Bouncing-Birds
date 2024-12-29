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

    // 说明页面相关
    bool showInstructions;                    // 控制说明页面显示
    sf::RectangleShape instructionBackground; // 说明页面背景
    std::vector<sf::Text> instructionLines;   // 说明文本行
    sf::Font font;                            // 字体

    // 添加滚动相关变量
    float scrollOffset;          // 文本滚动偏移量
    float scrollSpeed;           // 滚动速度
    float maxScrollOffset;       // 最大滚动范围

    sf::Text helpPrompt;  // 添加提示文本成员变量

    // 处理输入事件
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // P 键事件处理 - 只在第二页时有效
            if (currentPage == 2 && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::P) {
                    showInstructions = !showInstructions;
                    std::cout << "Instructions toggled: " << (showInstructions ? "shown" : "hidden") << std::endl;  // 调试输出
                }
            }

            // 现有的鼠标点击事件处理
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (button && button->isClicked(sf::Mouse::getPosition(window)) && currentPage == 1) {
                    background.loadTexture("Images/second_page.png");
                    background.scaleSprite(window);
                    startTransition();
                    currentPage = 2;
                    button.reset();
                } else if (button2 && button2->isClicked(sf::Mouse::getPosition(window)) && currentPage == 2) {
                    background.loadTexture("Images/background.png");
                    background.scaleSprite(window);
                    startTransition();
                    currentPage = 3;
                    button2.reset();
                    switchToGame();
                }
            }

            // 添加鼠标滚轮事件处理
            if (currentPage == 2 && showInstructions && event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    // 向上滚动是负值，向下滚动是正值，所以要取反
                    float delta = -event.mouseWheelScroll.delta * scrollSpeed;
                    
                    // 更新滚动偏移量，并确保在有效范围内
                    scrollOffset = std::clamp(scrollOffset + delta, 0.f, maxScrollOffset);
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
            return;
        }

        renderTexture.clear(sf::Color::Transparent);
        background.draw(renderTexture);
        
        // 在第二页时显示按钮和提示文本
        if (currentPage == 2) {
            if (button2) {
                button2->draw(renderTexture, isTransitioning ? alpha : 255.0f);
            }

            // 设置并绘制提示文本
            helpPrompt.setString(L"按 P 键查看游戏说明");
            helpPrompt.setCharacterSize(72);  
            helpPrompt.setCharacterSize(72);
            helpPrompt.setFillColor(sf::Color(25, 25, 112));  
            
            
            // 调整位置到按钮附近
            helpPrompt.setPosition(840, 820); 
            
            renderTexture.draw(helpPrompt);
        } else if (currentPage == 1 && button) {
            button->draw(renderTexture, 255.0f);
        }

        renderTexture.display();

        sf::Sprite renderSprite(renderTexture.getTexture());
        window.clear();
        window.draw(renderSprite);

        if (isTransitioning) {
            alpha = Easing::quadraticEaseInOut(currentTime, 255.0f, -255.0f, transitionTime);
            sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
            overlay.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha)));
            window.draw(overlay);
        }

        // 在第二页且需要显示说明时绘制说明页面
        if (currentPage == 2 && showInstructions) {
            window.draw(instructionBackground);
            
            // 获取说明背景的边界作为剪切区域
            float topY = instructionBackground.getPosition().y;
            float bottomY = topY + instructionBackground.getSize().y;
            
            // 绘制文本
            for (const auto& text : instructionLines) {
                sf::Text scrolledText = text;
                float newY = text.getPosition().y - scrollOffset;
                scrolledText.setPosition(text.getPosition().x, newY);
                
                // 只绘制在背景区域内的文本
                float textTop = scrolledText.getPosition().y;
                float textBottom = textTop + scrolledText.getCharacterSize();
                
                if (textBottom >= topY+50 && textTop <= bottomY-50) {
                    window.draw(scrolledText);
                }
            }
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

    void initializeInstructions() {
        std::vector<std::wstring> lines = {
            L"哐哐当当雀雀球 - 操作指南",
            L"",
            L"---基础操作---",
            L"",
            L"鼠标操作",
            L"按住左键蓄力瞄准",
            L"松开左键发射小鸟",
            L"移动鼠标调整角度",
            L"",
            L"键盘操作",
            L"1-4 数字键选择不同小鸟",
            L"[R] 读取存档",
            L"[S] 保存进度",
            L"[V] 切换结束/回放",
            L"[ESC] 暂停游戏",
            L"",
            L"---小鸟图鉴---",
            L"基础小鸟 (1号)",
            L"轻巧灵活，适合新手",
            L"进阶小鸟 (2号)",
            L"稳定性好，反弹适中",
            L"超能小鸟 (3号)",
            L"范围推动，群体清理",
            L"王牌小鸟 (4号)",
            L"终极推动，清场利器",
            L"",
            L"---游戏提示---",
            L"合理规划出击顺序",
            L"善用场地边界反弹",
            L"把握蓄力时机",
            L"",
            L"Made by Hanwen Zhang",
            L"按 [P] 关闭说明"
        };

        float startY = instructionBackground.getPosition().y + 50;
        float contentWidth = instructionBackground.getSize().x - 100;

        for (const auto& line : lines) {
            sf::Text text;
            text.setFont(font);
            text.setString(line);
            

            text.setCharacterSize(48);  
            text.setFillColor(sf::Color::White);
            
            // 不同类型文本的样式
            if (line.find(L"哐哐当当") != std::wstring::npos) {
                text.setCharacterSize(72);
                text.setFillColor(sf::Color::Yellow);
                text.setStyle(sf::Text::Bold);
            } else if (line.find(L"---") != std::wstring::npos) {
                text.setCharacterSize(60);
                text.setFillColor(sf::Color(30, 144, 255));
                text.setStyle(sf::Text::Bold);
            } else {
                text.setFillColor(sf::Color::White);
            }
            
            // 计算文本居中位置
            float textWidth = text.getLocalBounds().width;
            float xPosition = instructionBackground.getPosition().x + (instructionBackground.getSize().x - textWidth) / 2;
            
            text.setPosition(xPosition, startY);
            
            instructionLines.push_back(text);
            startY += text.getCharacterSize() + 20;
        }

        // 计算最大滚动范围
        float visibleHeight = instructionBackground.getSize().y - 100;
        maxScrollOffset = std::max(0.f, startY - 
                                      (instructionBackground.getPosition().y + visibleHeight));
        scrollOffset = 0.f;
        scrollSpeed = 40.f;
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
              alpha(255.0f),
              showInstructions(false),
              scrollOffset(0.f),
              scrollSpeed(30.f),
              maxScrollOffset(0.f)  // 将在 initializeInstructions 中计算
    {
        // 加载并设置窗口图标
       if (icon.loadFromFile("Images/bird_2.png")) {
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

        // 加载字体
        if (!font.loadFromFile("chinese.ttf")) {
            throw std::runtime_error("Failed to load font!");
        }

        // 初始化说明页面背景
        instructionBackground.setSize(sf::Vector2f(1200, 800));
        instructionBackground.setFillColor(sf::Color(100, 200, 100, 250));
        instructionBackground.setPosition(
            (1920 - 1200) / 2,  // 居中显示
            (1080 - 800) / 2
        );

        // 初始化说明文本
        initializeInstructions();

        // 初始化提示文本
        helpPrompt.setFont(font);
        helpPrompt.setCharacterSize(48);
        helpPrompt.setFillColor(sf::Color::White);
        helpPrompt.setString(L"按 P 键查看游戏说明");
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