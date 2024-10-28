//
// Created by wangmingdong on 2023/6/28.
//

#include <iostream>

#include "SFML\Graphics.hpp"
#include "SFML\Window.hpp"
#include "SFML\System.hpp"

using namespace sf;

int main() {

    // 创建窗口对象
    RenderWindow window(VideoMode(800, 600), "Fighters", Style::Default);

    // 显示刷新速度
    window.setFramerateLimit(60);

    // 创建一个圆
    CircleShape shape(50.0f);

    // 主循环
    // 目的：1. 让程序一直运行  2. 处理事件（键盘、鼠标操作等）
    while (window.isOpen()) {

        // 事件处理
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }
        // 如果按下ESC那么就退出
        if (Keyboard::isKeyPressed(Keyboard::Escape))
            window.close();
        if (Keyboard::isKeyPressed(Keyboard::Up))
            // 键盘 上
            shape.move(0.0f, -2.0f);
        if (Keyboard::isKeyPressed(Keyboard::Down))
            // 键盘 下
            shape.move(0.0f, 2.0f);
        if (Keyboard::isKeyPressed(Keyboard::Left))
            // 键盘 左
            shape.move(-2.0f, 0.f);
        if (Keyboard::isKeyPressed(Keyboard::Right))
            // 键盘 右
            shape.move(2.0f, 0.0f);

        // 清空窗口上一次显示的内容，类似 用黑板擦 将黑板上之前写的内容清楚
        window.clear(Color::Red);

        // 将这个圆显示到窗口中
        window.draw(shape);

        // 显示窗口中的内容
        window.display();
    }

    return 0;
}
