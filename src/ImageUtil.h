#pragma once
#include <SFML/Graphics.hpp>

// Downscale 'src' to exactly (targetW x targetH) using nearest-neighbor.
// This reduces VRAM massively before we upload to sf::Texture.
inline sf::Image downscaleNearest(const sf::Image& src, unsigned targetW, unsigned targetH) {
    sf::Image out;
    out.create(targetW, targetH, sf::Color::Transparent);

    const auto sW = src.getSize().x;
    const auto sH = src.getSize().y;
    if (sW == 0 || sH == 0) return out;

    for (unsigned y = 0; y < targetH; ++y) {
        unsigned sy = static_cast<unsigned>((uint64_t)y * sH / targetH);
        for (unsigned x = 0; x < targetW; ++x) {
            unsigned sx = static_cast<unsigned>((uint64_t)x * sW / targetW);
            out.setPixel(x, y, src.getPixel(sx, sy));
        }
    }
    return out;
}
