#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/*
 * Changes from your version:
 * 1) Inherit PUBLICLY from sf::NonCopyable (was private by default).
 * 2) Make the destructor virtual, so deleting via base pointer is safe.
 * 3) Keep the exact virtual method signatures that children will override.
 */
class AGameObject : public sf::NonCopyable
{
public:
    using String = std::string;

    explicit AGameObject(String name);
    virtual ~AGameObject();  // << make virtual

    // --- virtual interface children override ---
    virtual void initialize() = 0;
    virtual void processInput(sf::Event event) = 0;
    virtual void update(sf::Time deltaTime) = 0;
    virtual void draw(sf::RenderWindow* targetWindow); // non-pure but virtual

    // --- helpers (leave virtual if children customize) ---
    virtual void setPosition(float x, float y);
    virtual void setScale(float x, float y);
    virtual sf::FloatRect getLocalBounds();
    virtual sf::Vector2f getPosition();
    virtual sf::Vector2f getScale();

    String getName();

    // z-order for rendering. Higher z means drawn later (on top).
    void setZOrder(int z) { zOrder = z; }
    int getZOrder() const { return zOrder; }

protected:
    String       name;
    sf::Sprite* sprite = new sf::Sprite();
    sf::Texture* texture = new sf::Texture();

    float posX = 0.0f, posY = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f;

    int zOrder = 0; // default layer
};
