#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;
constexpr float PLAYER_RADIUS = 20.f;
constexpr float ENEMY_RADIUS = 15.f;
constexpr float REBOUND_COEFFICIENT = 0.8f;
constexpr float FRICTION_COEFFICIENT = 0.98f;
const sf::Vector2f CENTER_ZONE_POSITION(710, 290);
constexpr int NUM_ENEMIES = 6;

class GameObject {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool isStopped;

    GameObject(float radius, sf::Color color, sf::Vector2f position)
            : velocity(0.f, 0.f), isStopped(true) {
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setOrigin(radius, radius);
        shape.setPosition(position);
    }

    void updatePosition(float dt) {
        if (!isStopped) {
            shape.move(velocity * dt);
            velocity *= FRICTION_COEFFICIENT;  // 应用摩擦力
            if (std::abs(velocity.x) < 0.01f && std::abs(velocity.y) < 0.01f) {
                isStopped = true;
                velocity = {0.f, 0.f};
            }
        }
    }

    void applyBoundaryCollision() {
        auto pos = shape.getPosition();
        if (pos.x - shape.getRadius() < 0 || pos.x + shape.getRadius() > WINDOW_WIDTH) {
            velocity.x = -velocity.x * REBOUND_COEFFICIENT;
        }
        if (pos.y - shape.getRadius() < 0 || pos.y + shape.getRadius() > WINDOW_HEIGHT) {
            velocity.y = -velocity.y * REBOUND_COEFFICIENT;
        }
    }
};

class CollisionHandler {
public:
    static void applyCollision(GameObject& a, GameObject& b, sf::Sound& collisionSound) {
        auto delta = a.shape.getPosition() - b.shape.getPosition();
        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (dist < a.shape.getRadius() + b.shape.getRadius()) {
            collisionSound.play();
            auto normal = delta / dist;

            // 计算相对速度
            auto relativeVelocity = a.velocity - b.velocity;
            float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

            // 仅在相向碰撞前处理
            if (velocityAlongNormal > 0) return;

            // 冲量计算
            float j = -(1 + REBOUND_COEFFICIENT) * velocityAlongNormal;

            // 更新速度
            a.velocity += j * normal;
            b.velocity -= j * normal;

            // 确保没有重叠
            float penetration = (a.shape.getRadius() + b.shape.getRadius()) - dist;
            sf::Vector2f correction = normal * (penetration / 2.0f);
            a.shape.move(-correction);
            b.shape.move(correction);
        }
    }
};

class Game {
private:
    sf::RenderWindow window;
    sf::SoundBuffer collisionBuffer;
    sf::Sound collisionSound;
    std::vector<GameObject> enemies;
    std::vector<GameObject> players;

    int selectedPlayerIndex;
    bool isCharging;
    float chargeTime;

public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Game"),
             selectedPlayerIndex(-1), isCharging(false), chargeTime(0.f) {
        window.setFramerateLimit(60);
        collisionBuffer.loadFromFile("collision.wav");
        collisionSound.setBuffer(collisionBuffer);

        initializePlayers();
        initializeEnemies();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            updateGameObjects();
            checkCollisions();
            render();
        }
    }

private:
    void initializeEnemies() {
        for (int i = 0; i < NUM_ENEMIES; ++i) {
            float x = CENTER_ZONE_POSITION.x + std::rand() % static_cast<int>(WINDOW_WIDTH - 70);
            float y = CENTER_ZONE_POSITION.y + std::rand() % static_cast<int>(WINDOW_HEIGHT - 70);
            enemies.emplace_back(ENEMY_RADIUS, sf::Color::Red, sf::Vector2f(x, y));
        }
    }

    void initializePlayers() {
        players.emplace_back(PLAYER_RADIUS, sf::Color::Green, sf::Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50));
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (selectedPlayerIndex != -1) {
                    isCharging = true;
                    chargeTime = 0.f;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && isCharging) {
                sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
                launchPlayer(mousePos);
                isCharging = false;
            }
        }
    }

    void launchPlayer(const sf::Vector2f& mousePos) {
        if (selectedPlayerIndex >= 0) {
            auto& player = players[selectedPlayerIndex];
            sf::Vector2f direction = mousePos - player.shape.getPosition();
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (length > 0) {
                direction /= length;
            }

            player.velocity = direction * 250.f;  // 速度设置
            player.isStopped = false;
        }
    }

    void updateGameObjects() {
        for (auto& enemy : enemies) enemy.updatePosition(0.1f);
        for (auto& player : players) player.updatePosition(0.1f);
    }

    void checkCollisions() {
        for (auto& player : players) {
            player.applyBoundaryCollision();
            for (auto& enemy : enemies) {
                enemy.applyBoundaryCollision();
                CollisionHandler::applyCollision(player, enemy, collisionSound);
            }
        }
    }

    void render() {
        window.clear();
        for (const auto& enemy : enemies) window.draw(enemy.shape);
        for (const auto& player : players) window.draw(player.shape);
        window.display();
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}