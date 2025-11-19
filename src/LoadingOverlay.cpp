#include "LoadingOverlay.h"
#include "BatchAssetLoader.h"

LoadingOverlay::LoadingOverlay(BatchAssetLoader* loader, const sf::Font* font)
    : AGameObject("LoadingOverlay"), loader_(loader), font_(font)
{
}

LoadingOverlay::~LoadingOverlay() = default;

void LoadingOverlay::initialize() {
    // full-window blue backdrop
    backdrop_.setFillColor(sf::Color(0x57, 0x65, 0xF0, 255));
    text_.setFillColor(sf::Color::White);
    text_.setOutlineColor(sf::Color::Black);
    text_.setOutlineThickness(3.f);
    text_.setCharacterSize(40);
    if (font_) text_.setFont(*font_);
    //text_.setString("Loading...");
}

void LoadingOverlay::processInput(sf::Event event) {
    // swallow input when visible (optional)
}

void LoadingOverlay::updateTipText() {
    if (tips_.empty()) return;
    text_.setString(tips_[tipIndex_]);
    auto lb = text_.getLocalBounds();
    text_.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
}

void LoadingOverlay::update(sf::Time deltaTime) {
    // rotate tip every tipIntervalSec_
    if (tips_.size() > 0) {
        if (tipClock_.getElapsedTime().asSeconds() >= tipIntervalSec_) {
            tipIndex_ = (tipIndex_ + 1) % tips_.size();
            updateTipText();
            tipClock_.restart();
        }
    }

    // nothing heavy; only update text when loader present
    if (!loader_) return;
    if (!loader_->isFinished()) {
        int uploaded = loader_->getUploaded();
        int total = loader_->getTotal();
        float pct = loader_->getUploadedPercent();
        std::string progress = " (" + std::to_string(static_cast<int>(pct)) + "% - "
            + std::to_string(uploaded) + "/" + std::to_string(total) + ")";

        // if tips exist, append the progress suffix to the tip text
        if (!tips_.empty()) {
            std::string base = tips_[tipIndex_];
            text_.setString(base + progress);
        } else {
            text_.setString("Loading... " + progress);
        }
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

void LoadingOverlay::setTips(const std::vector<std::string>& tips) {
    tips_ = tips;
    tipIndex_ = 0;
    tipClock_.restart();
    updateTipText();
}
