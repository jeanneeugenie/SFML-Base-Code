#include "BaseRunner.h"
#include "GameObjectManager.h"
#include "BGObject.h"
#include "TextureManager.h"
#include "TextureDisplay.h"
#include "FPSCounter.h"

// batch loading
#include "BatchAssetLoader.h"
#include "TextureSink.h"
#include <filesystem>
#include <iostream>

// ------------------------------------------------------------------
const sf::Time BaseRunner::TIME_PER_FRAME = sf::seconds(1.f / 60.f);

std::unique_ptr<BatchAssetLoader> g_loader;
TextureSink g_sink;

// --- Small UI toast for batch notifications (file-static) ---
static sf::Font   g_toastFont;
static sf::Text   g_toastText;
static sf::Clock  g_toastClock;
static int        g_lastShownBatch = -1;
// ------------------------------------------------------------------

BaseRunner::BaseRunner() :
    window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "HO: Entity Component", sf::Style::Close)
{
    // Cap render FPS to ~120 so the counter isn't in the 300s
    window.setVerticalSyncEnabled(false);
    window.setFramerateLimit(120);

    // 1) Synchronous base assets
    TextureManager::getInstance()->loadFromAssetList();

    // 2) Scene objects
    auto* bgObject = new BGObject("BGObject");
    GameObjectManager::getInstance()->addObject(bgObject);

    auto* display = new TextureDisplay();
    GameObjectManager::getInstance()->addObject(display);

    auto* fpsCounter = new FPSCounter();
    GameObjectManager::getInstance()->addObject(fpsCounter);

    // 3) Gather streaming files
    std::vector<std::string> streamingFiles;
    for (auto& entry : std::filesystem::directory_iterator("Media/Streaming")) {
        if (!entry.is_regular_file()) continue;
        auto path = entry.path().string();
        if (path.ends_with(".png") || path.ends_with(".jpg"))
            streamingFiles.push_back(path);
    }

    // Loader: try batchSize=15 or 50 with interval 120–250 ms
    g_loader = std::make_unique<BatchAssetLoader>(
        &g_sink,
        streamingFiles,
        /*batchSize*/ 50,
        /*intervalMs*/ 200,  // slightly longer to make batches visually distinct
        /*workerThreads*/ 4,
        /*streaming*/ true
    );

    // --- Initialize toast UI (top-left) ---
    g_toastFont.loadFromFile("Media/Sansation.ttf");
    g_toastText.setFont(g_toastFont);
    g_toastText.setCharacterSize(22);
    g_toastText.setFillColor(sf::Color::White);
    g_toastText.setOutlineColor(sf::Color::Black);
    g_toastText.setOutlineThickness(2.f);
    g_toastText.setPosition(12.f, 10.f);
    g_toastText.setString("");

    // Show a toast briefly when uploads from a new batch arrive
    g_loader->setOnUpload([](int batchId) {
        if (batchId > g_lastShownBatch) {
            g_lastShownBatch = batchId;
            g_toastText.setString("Batch " + std::to_string(batchId) + " uploading...");
            g_toastClock.restart();
        }
        });

    std::cout << "[BaseRunner] Streaming files: " << streamingFiles.size() << "\n";
}

void BaseRunner::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (this->window.isOpen())
    {
        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;

        while (timeSinceLastUpdate > TIME_PER_FRAME)
        {
            timeSinceLastUpdate -= TIME_PER_FRAME;
            processEvents();
            update(elapsedTime);
        }
        render();
    }
}

void BaseRunner::processEvents()
{
    sf::Event event;
    while (this->window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            this->window.close();
        else
            GameObjectManager::getInstance()->processInput(event);
    }
}

void BaseRunner::update(sf::Time elapsedTime)
{
    GameObjectManager::getInstance()->update(elapsedTime);

    if (g_loader) {
        g_loader->update();                // schedules batches on timer
        g_loader->drainToTextures(10);     // uploads per frame (tune 6–20)
    }
}

void BaseRunner::render()
{
    this->window.clear();
    GameObjectManager::getInstance()->draw(&this->window);

    // Draw batch toast for ~0.6s after a new batch starts uploading
    if (g_toastClock.getElapsedTime().asSeconds() < 0.6f) {
        this->window.draw(g_toastText);
    }

    this->window.display();
}
