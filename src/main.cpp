//
// Created by wangmingdong on 2023/6/28.
//

#include <iostream>

#include "SFML\Graphics.hpp"
#include "SFML\Window.hpp"
#include "SFML\System.hpp"

using namespace std;
using namespace sf;

class Bullet : public Sprite {
private:
public:
    Bullet(float x, float y) {
        Texture *bullet_img = new Texture();
        if (!bullet_img->loadFromFile("Images/missileTex01.png")) {
            throw "Could not load png!";
        } else {
            this->setTexture(*bullet_img);
            this->setScale(Vector2f(0.15f, 0.15f));
            this->setPosition(Vector2f(x, y));
        }
    }

    ~Bullet() {

    }
};

class PlayerPlane : public Sprite {
private:
    RenderWindow &window;
    // 声明静态成员变量，一定要在类的外部进行定义
    static std::vector<Sprite> bullets;
    long int last_create_bullet_time;
public:
    PlayerPlane(RenderWindow &window) : window(window) {
        Texture *player_plane_img = new Texture();
        if (!player_plane_img->loadFromFile("Images/ship.png")) {
            throw "Could not load png!";
        } else {
            this->setTexture(*player_plane_img);
            this->setScale(Vector2f(0.15f, 0.15f));
        }
        // 创建子弹的间隔事件
        this->last_create_bullet_time = time(NULL);
    }

    ~PlayerPlane() {

    }

    void key_handle() {
        // 如果按下ESC那么就退出
        if (Keyboard::isKeyPressed(Keyboard::Escape))
            this->window.close();
        if (Keyboard::isKeyPressed(Keyboard::Up))
            // 键盘 上
            this->move(0.0f, -4.0f);
        if (Keyboard::isKeyPressed(Keyboard::Down))
            // 键盘 下
            this->move(0.0f, 4.0f);
        if (Keyboard::isKeyPressed(Keyboard::Left))
            // 键盘 左
            this->move(-4.0f, 0.f);
        if (Keyboard::isKeyPressed(Keyboard::Right))
            // 键盘 右
            this->move(4.0f, 0.0f);
        if (Keyboard::isKeyPressed(Keyboard::Space)) {
            // 空格键
            long int current_time = time(NULL);
            if (current_time > this->last_create_bullet_time) {
                cout << "开火\n";
                this->fire();
                this->last_create_bullet_time = current_time;
            }
        }

    }

    void fire() {
        this->bullets.push_back(Bullet(this->getPosition().x, this->getPosition().y));
    }

    void display() {
        // 显示飞机
        this->window.draw(*this);
        // 显示所有的子弹
        for (size_t i = 0; i < this->bullets.size(); i++) {
            this->window.draw(this->bullets[i]);
        }
    }
};

class Enemy : public Sprite {
private:
    // 声明静态成员变量，一定要在类的外部进行定义
    static std::vector<Sprite> enemies;
    // 创建敌机的间隔时间
    int enemy_create_time;
    // 窗口window
    RenderWindow &window;
public:
    Enemy(RenderWindow &window) : window(window) {
        Texture *enemy_img = new Texture();
        if (!enemy_img->loadFromFile("Images/enemy.png")) {
            throw "Could not load png!";
        } else {
            this->setTexture(*enemy_img);
            this->setScale(Vector2f(0.15f, 0.15f));
        }

        // 拷贝构造一个新的敌机对象，添加到容器中
        // Enemy::enemies.push_back(Sprite(*this));  // 因为是类的静态成员变量，所以用 类名::成员变量 是可以调用的
        this->enemies.push_back(Sprite(*this));

        // 设置创建敌机间隔事件
        this->enemy_create_time = 15;
    }

    ~Enemy() {

    }

    void create_enemy() {
        // 每隔一段时间，创建1个新的敌机对象
        if (this->enemy_create_time < 40) {
            this->enemy_create_time++;
        }
        if (this->enemy_create_time >= 40) {
            // 修改第1架敌机的位置
            this->setPosition(this->window.getSize().x, rand() % int(window.getSize().y - this->getGlobalBounds().height));
            // 拷贝构造第1架敌机对象
            enemies.push_back(Sprite(*this));
            this->enemy_create_time = 0;
        }
    }

    void move_enemy() {
        // 移动所有的敌机
        for (size_t i = 0; i < this->enemies.size(); i++) {
            // 向左移动敌机
            this->enemies[i].move(-7.f, 0.f);
            // 如果已经移动到左侧看不到的位置，那么就删除
            if (this->enemies[i].getPosition().x < 0 - this->enemies[i].getGlobalBounds().width) {
                this->enemies.erase(this->enemies.begin() + i);
            }
        }
    }

    void display() {
        // 将多架敌机显示到窗口中
        for (size_t i = 0; i < this->enemies.size(); i++) {
            this->window.draw(this->enemies[i]);
        }
    }
};

// 定义静态成员变量
std::vector<Sprite> Enemy::enemies;
std::vector<Sprite> PlayerPlane::bullets;

int main() {

    // 创建窗口对象
    RenderWindow window(VideoMode(1000, 800), "Fighters", Style::Default);

    // 显示刷新速度
    window.setFramerateLimit(60);

    // 创建精灵对象(玩家飞机)
    PlayerPlane player_plane(window);

    // 创建精灵对象(敌机对象)
    Enemy enemy(window);

    // 主循环
    // 目的：1. 让程序一直运行  2. 处理事件（键盘、鼠标操作等）
    while (window.isOpen()) {

        // 事件处理
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        // 玩家飞机键盘控制处理
        player_plane.key_handle();

        // 创建敌机对象
        enemy.create_enemy();

        // 移动敌机对象
        enemy.move_enemy();

        // 清空窗口上一次显示的内容，类似 用黑板擦 将黑板上之前写的内容清楚
        window.clear(Color::Black);

        // 将玩家飞机、发射的子弹显示到窗口中
        player_plane.display();

        // 将敌机飞机显示到窗口中
        enemy.display();

        // 显示窗口中的内容
        window.display();
    }

    return 0;
}
