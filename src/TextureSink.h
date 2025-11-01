#pragma once
#include "BatchAssetLoader.h"
#include "TextureManager.h"

/*
 * TextureSink
 * ------------
 * Bridges the background loader to the TextureManager.
 * Runs on the main thread: converts sf::Image -> sf::Texture.
 */
class TextureSink final : public ITextureSink {
public:
    void createTextureFromImage(const DecodedImage& di) override {
        TextureManager::getInstance()->instantiateFromImage(di.assetName, di.image, di.streaming);
    }
};
