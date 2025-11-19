#pragma once

#include "AGameObject.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class BatchAssetLoader; // forward

class LoadingOverlay : public AGameObject {
public:
    LoadingOverlay(BatchAssetLoader* loader, const sf::Font* font = nullptr);
    ~LoadingOverlay() override;

    void initialize() override;
    void processInput(sf::Event event) override;
    void update(sf::Time deltaTime) override;
    void draw(sf::RenderWindow* targetWindow) override;

    // Tooltip tips: set the rotating strings. Rotation restarts at index 0.
    void setTips(const std::vector<std::string>& tips);
    void setTipIntervalSeconds(float seconds) { tipIntervalSec_ = seconds; }

private:
    void updateTipText();

    BatchAssetLoader* loader_ = nullptr;
    const sf::Font* font_ = nullptr;
    sf::RectangleShape backdrop_;
    sf::Text text_;

    std::vector<std::string> tips_;
    size_t tipIndex_ = 0;
    sf::Clock tipClock_;
    float tipIntervalSec_ = 5.f; // rotate every 5s
};
