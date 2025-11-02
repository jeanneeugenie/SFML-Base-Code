#pragma once
#include <SFML/Graphics.hpp>
#include "AGameObject.h"

class FPSCounter : public AGameObject {
public:
    FPSCounter();
    ~FPSCounter() override;

    void initialize() override;
    void processInput(sf::Event event) override;
    void update(sf::Time deltaTime) override;
    void draw(sf::RenderWindow* targetWindow) override;

private:
    void updateFPS(sf::Time elapsedTime);

    // --- state ---
    sf::Text* statsText = nullptr;
    sf::Time   updateTime = sf::Time::Zero;
    int        framesPassed = 0;

    // NEW: smoothed fps value (fixes “fpsEMA is not a member”)
    float      fpsEMA = 60.0f;
};
