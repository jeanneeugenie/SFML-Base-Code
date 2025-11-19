#include "BaseRunner.h"
#include "GameObjectManager.h"
#include "BGObject.h"
#include "TextureManager.h"
#include "TextureDisplay.h"
#include "FPSCounter.h"
#include "LoadingGif.h"
#include "LoadingOverlay.h"

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

// Keep a pointer to the overlay object so we can add/remove it
static LoadingOverlay* g_loadingOverlay = nullptr;

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
    TextureManager::getInstance()->loadGIF();

    // 2) Scene objects
    BGObject* bgObject = new BGObject("BGObject");
    GameObjectManager::getInstance()->addObject(bgObject);

    TextureDisplay* display = new TextureDisplay();
    GameObjectManager::getInstance()->addObject(display);

    FPSCounter* fpsCounter = new FPSCounter();
    GameObjectManager::getInstance()->addObject(fpsCounter);

    LoadingGif* loadingGIF = new LoadingGif("LoadingGif");
    GameObjectManager::getInstance()->addObject(loadingGIF);

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
        /*batchSize*/ 60,
        /*intervalMs*/ 120,  // slightly longer to make batches visually distinct
        /*workerThreads*/ 4,
        /*streaming*/ true
    );

    // Give the loading GIF a pointer to the loader so it can follow progress
    loadingGIF->setLoader(g_loader.get());

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
    /*g_loader->setOnUpload([](int batchId) {
        if (batchId > g_lastShownBatch) {
            g_lastShownBatch = batchId;
            g_toastText.setString("Batch " + std::to_string(batchId) + " uploading...");
            g_toastClock.restart();
        }
        });*/

    // Create overlay object and add it to the scene so it renders on top
    g_loadingOverlay = new LoadingOverlay(g_loader.get(), &g_toastFont);
    GameObjectManager::getInstance()->addObject(g_loadingOverlay);

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
    // Update loader first so scene objects can react to the newest upload progress
    if (g_loader) {
        g_loader->update();                // schedules batches on timer
        g_loader->drainToTextures(10);     // uploads per frame (tune 6–20)

        // If loader finished, remove overlay object from scene
        if (g_loadingOverlay && g_loader->isFinished()) {
            GameObjectManager::getInstance()->deleteObject(g_loadingOverlay);
            g_loadingOverlay = nullptr;
        }
    }

    GameObjectManager::getInstance()->update(elapsedTime);
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
