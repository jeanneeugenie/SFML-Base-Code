#pragma once
#include "BatchAssetLoader.h"
#include "TextureManager.h"
#include "ImageUtil.h"

// Bridge decoded images -> TextureManager, with pre-upload downscale to save VRAM.
class TextureSink final : public ITextureSink {
public:
    void createTextureFromImage(const DecodedImage& di) override {
        // Pick your icon size; 64x64 is typical for UI tiles.
        static constexpr unsigned ICON_W = 64;
        static constexpr unsigned ICON_H = 64;

        // Downscale on CPU, then upload the small image.
        sf::Image small = downscaleNearest(di.image, ICON_W, ICON_H);
        TextureManager::getInstance()->instantiateFromImage(di.assetName, small, di.streaming);
    }
};
