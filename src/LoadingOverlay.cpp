#include "LoadingOverlay.h"
#include "BatchAssetLoader.h"

LoadingOverlay::LoadingOverlay(BatchAssetLoader* loader, const sf::Font* font)
    : AGameObject("LoadingOverlay"), loader_(loader), font_(font)
{
}

LoadingOverlay::~LoadingOverlay() = default;

void LoadingOverlay::initialize() {
    backdrop_.setFillColor(sf::Color(0x57, 0x65, 0xF0, 255));
    text_.setFillColor(sf::Color::White);
    text_.setOutlineColor(sf::Color::Black);
    text_.setOutlineThickness(3.f);
    text_.setCharacterSize(40);
    if (font_) text_.setFont(*font_);
    text_.setString("Loading...");
}

void LoadingOverlay::processInput(sf::Event event) {
    // swallow input when visible (optional)
}

void LoadingOverlay::update(sf::Time deltaTime) {
    // nothing heavy; only update text when loader present
    if (!loader_) return;
    if (!loader_->isFinished()) {
        int uploaded = loader_->getUploaded();
        int total = loader_->getTotal();
        float pct = loader_->getUploadedPercent();
        std::string progress = "Loading... (" + std::to_string(static_cast<int>(pct)) + "% - "
            + std::to_string(uploaded) + "/" + std::to_string(total) + ")";
        text_.setString(progress);
        auto lb = text_.getLocalBounds();
        text_.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        // position will be set in draw because we need window size
    }
}

void LoadingOverlay::draw(sf::RenderWindow* targetWindow) {
    if (!loader_ || loader_->isFinished()) return; // only draw while loading

    sf::Vector2u s = targetWindow->getSize();
    backdrop_.setSize({(float)s.x, (float)s.y});
    text_.setPosition(s.x / 2.f, s.y / 2.f);

    targetWindow->draw(backdrop_);
    targetWindow->draw(text_);
}
