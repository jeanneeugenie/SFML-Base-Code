#pragma once

#include "AGameObject.h"
#include <SFML/Graphics.hpp>

class BatchAssetLoader; // forward

class LoadingOverlay : public AGameObject {
public:
    LoadingOverlay(BatchAssetLoader* loader, const sf::Font* font = nullptr);
    ~LoadingOverlay() override;

    void initialize() override;
    void processInput(sf::Event event) override;
    void update(sf::Time deltaTime) override;
    void draw(sf::RenderWindow* targetWindow) override;

private:
    BatchAssetLoader* loader_ = nullptr;
    const sf::Font* font_ = nullptr;
    sf::RectangleShape backdrop_;
    sf::Text text_;
};
