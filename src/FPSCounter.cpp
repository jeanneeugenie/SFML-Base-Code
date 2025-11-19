#include "FPSCounter.h"
#include <iostream>
#include "BaseRunner.h"

namespace {
    // update every 250 ms instead of 1s
    constexpr float UPDATE_INTERVAL_SEC = 0.25f;
    // smoothing for FPS number (EMA)
    constexpr float FPS_EMA_ALPHA = 0.25f; // 0=no smoothing, 1=very smooth
}

FPSCounter::FPSCounter() : AGameObject("FPSCounter") {}

FPSCounter::~FPSCounter()
{
    delete this->statsText->getFont();
    delete this->statsText;
    AGameObject::~AGameObject();
}

void FPSCounter::initialize()
{
    // set high z so FPS displays above overlays
    this->setZOrder(200);

    // load font once
    sf::Font* font = new sf::Font();
    if (!font->loadFromFile("Media/Sansation.ttf")) {
        std::cerr << "[FPSCounter] Failed to load font Media/Sansation.ttf\n";
    }

    this->statsText = new sf::Text();
    this->statsText->setFont(*font);

    // keep a little padding from the bottom-right so it doesn't clip
    const float MARGIN_X = 18.f;
    const float MARGIN_Y = 18.f;
    this->statsText->setPosition(
        BaseRunner::WINDOW_WIDTH - 150.f - MARGIN_X,
        BaseRunner::WINDOW_HEIGHT - 60.f - MARGIN_Y
    );

    this->statsText->setFillColor(sf::Color::White);
    this->statsText->setOutlineColor(sf::Color::Black);
    this->statsText->setOutlineThickness(2.0f);
    this->statsText->setCharacterSize(35);

    // show something immediately instead of waiting 1s
    this->statsText->setString("FPS: --");

    // init EMA with a reasonable guess
    this->fpsEMA = 60.f;
}

void FPSCounter::processInput(sf::Event) {}

void FPSCounter::update(sf::Time deltaTime)
{
    this->updateFPS(deltaTime);
}

void FPSCounter::draw(sf::RenderWindow* targetWindow)
{
    AGameObject::draw(targetWindow);
    if (this->statsText != nullptr)
        targetWindow->draw(*this->statsText);
}

void FPSCounter::updateFPS(sf::Time elapsedTime)
{
    // accumulate time and frames
    this->updateTime += elapsedTime;
    this->framesPassed++;

    // instantaneous fps for smoothing
    const float dt = elapsedTime.asSeconds();
    if (dt > 0.f) {
        const float inst = 1.f / dt;
        // EMA smoothing so number isn't jittery
        this->fpsEMA = (1.f - FPS_EMA_ALPHA) * this->fpsEMA + FPS_EMA_ALPHA * inst;
    }

    // refresh text more frequently (every 0.25s)
    if (this->updateTime.asSeconds() >= UPDATE_INTERVAL_SEC) {
        // average fps over the window, then blend a little with EMA for stability
        const float avg = this->framesPassed / this->updateTime.asSeconds();
        const float blended = 0.5f * avg + 0.5f * this->fpsEMA;

        this->statsText->setString("FPS: " + std::to_string(static_cast<int>(blended + 0.5f)));

        // reset for next mini-window
        this->updateTime -= sf::seconds(UPDATE_INTERVAL_SEC);
        this->framesPassed = 0;
    }
}
