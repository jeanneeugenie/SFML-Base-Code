#pragma once
#include "AGameObject.h"
#include <vector>
#include <SFML/Graphics.hpp>

/*
 * TextureDisplay
 * Dynamically mirrors streaming textures into a 15-column grid.
 */
class TextureDisplay : public AGameObject
{
public:
    TextureDisplay();
    ~TextureDisplay() override;

    // Signatures EXACTLY match AGameObject
    void initialize() override;
    void processInput(sf::Event event) override;
    void update(sf::Time deltaTime) override;
    void draw(sf::RenderWindow* targetWindow) override;

private:
    void addNewSpritesIfAny();
    void layoutSpriteAtIndex(size_t idx);

private:
    std::vector<sf::Sprite*> icons;
    size_t mirroredCount = 0;
};
