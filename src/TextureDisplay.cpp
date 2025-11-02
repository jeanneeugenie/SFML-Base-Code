#include "TextureDisplay.h"
#include "TextureManager.h"
#include <algorithm>

static constexpr int   GRID_COLS = 25;
static constexpr float TILE_SIZE = 64.f;
static constexpr float TILE_PAD = 4.f;
static const sf::Vector2f GRID_ORIGIN(16.f, 40.f);

static inline void fitSpriteToTile(sf::Sprite& spr, float tileSize = TILE_SIZE) {
    const sf::Texture* tex = spr.getTexture();
    if (!tex) return;
    sf::Vector2u sz = tex->getSize();
    if (sz.x == 0 || sz.y == 0) return;
    float s = tileSize / static_cast<float>(std::max(sz.x, sz.y));
    spr.setScale(s, s);
}

TextureDisplay::TextureDisplay() : AGameObject("TextureDisplay") {}

TextureDisplay::~TextureDisplay() {
    for (auto* s : icons) delete s;
    icons.clear();
}

void TextureDisplay::initialize() {
    // start empty; sprites are appended as textures arrive
}

void TextureDisplay::processInput(sf::Event) {}

void TextureDisplay::update(sf::Time) {
    addNewSpritesIfAny();
}

void TextureDisplay::draw(sf::RenderWindow* targetWindow) {
    AGameObject::draw(targetWindow);
    for (auto* s : icons) targetWindow->draw(*s);
}

void TextureDisplay::addNewSpritesIfAny()
{
    TextureManager* tm = TextureManager::getInstance();
    const size_t available = static_cast<size_t>(tm->getNumLoadedStreamTextures());

    while (mirroredCount < available) {
        sf::Texture* tex = tm->getStreamTextureFromList(static_cast<int>(mirroredCount));
        if (!tex) break;

        auto* spr = new sf::Sprite(*tex);
        fitSpriteToTile(*spr);
        icons.push_back(spr);

        layoutSpriteAtIndex(mirroredCount);
        ++mirroredCount;
    }
}

void TextureDisplay::layoutSpriteAtIndex(size_t idx)
{
    const int col = static_cast<int>(idx % GRID_COLS);
    const int row = static_cast<int>(idx / GRID_COLS);
    const float x = GRID_ORIGIN.x + col * (TILE_SIZE + TILE_PAD);
    const float y = GRID_ORIGIN.y + row * (TILE_SIZE + TILE_PAD);
    icons[idx]->setPosition(x, y);
}
