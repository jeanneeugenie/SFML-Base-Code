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

const sf::Time BaseRunner::TIME_PER_FRAME = sf::seconds(1.f / 60.f);

// globals owned by BaseRunner
std::unique_ptr<BatchAssetLoader> g_loader;
TextureSink g_sink;

// --- adaptive upload limiter state ---
static int   g_maxUploadsPerFrame = 8;   // initally 10, but 8 seems to work well and it will adapt to stay over 50 FPS
static float g_fpsEMA = 60.f; // simple smoothing of fps (exponential moving avg)
static const float FPS_TARGET_MIN = 50.f; // never drop below this (according to sir, this is the best number)
static const float FPS_RELAX_HIGH = 57.f; // if we’re above this, we can afford to upload faster

BaseRunner::BaseRunner() :
    window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "HO: Entity Component", sf::Style::Close)
{
    // Optional but helps stabilize
    // window.setVerticalSyncEnabled(true);
    // window.setFramerateLimit(60);

    // 1) Synchronous base assets
    TextureManager::getInstance()->loadFromAssetList();

    // 2) Scene objects
    auto* bgObject = new BGObject("BGObject");
    GameObjectManager::getInstance()->addObject(bgObject);

    auto* display = new TextureDisplay();       // <-- dynamic grid (see below)
    GameObjectManager::getInstance()->addObject(display);

    auto* fpsCounter = new FPSCounter();
    GameObjectManager::getInstance()->addObject(fpsCounter);

    // 3) Gather streaming files
    std::vector<std::string> streamingFiles;
    for (auto& entry : std::filesystem::directory_iterator("Media/Streaming"))
    {
        if (!entry.is_regular_file()) continue;
        auto path = entry.path().string();
        if (path.ends_with(".png") || path.ends_with(".jpg"))
            streamingFiles.push_back(path);
    }

    // 4) Init batch loader (15 per batch every 120ms; 4 workers)
    // 15 for testing will adjust for 50 later on
    // the thing lags at 15 making it 50 will :"D
    g_loader = std::make_unique<BatchAssetLoader>(
        &g_sink,
        streamingFiles,
        /*batchSize*/ 50,
        /*intervalMs*/ 300, //changing the time here allowed for some spacing?
        /*workerThreads*/ 4,
        /*streaming*/ true
    );

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

        // fixed timestep update
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
    while (this->window.pollEvent(event)) // handle all queued events
    {
        if (event.type == sf::Event::Closed) {
            this->window.close();
        }
        else {
            GameObjectManager::getInstance()->processInput(event);
        }
    }
}

void BaseRunner::update(sf::Time elapsedTime)
{
    GameObjectManager::getInstance()->update(elapsedTime);

    // --- Adaptive limiter based on FPS ---
    float instFPS = (elapsedTime.asSeconds() > 0.f) ? (1.f / elapsedTime.asSeconds()) : 1000.f;
    // EMA smoothing
    g_fpsEMA = 0.90f * g_fpsEMA + 0.10f * instFPS;

    // If FPS dips, back off uploads; if it’s comfy, increase a bit.
    if (g_fpsEMA < FPS_TARGET_MIN && g_maxUploadsPerFrame > 1) {
        g_maxUploadsPerFrame = std::max(1, g_maxUploadsPerFrame - 1);
    }
    else if (g_fpsEMA > FPS_RELAX_HIGH && g_maxUploadsPerFrame < 50) {
        g_maxUploadsPerFrame = std::min(50, g_maxUploadsPerFrame + 1);
    }

    // run batch loader every frame
    if (g_loader)
    {
        g_loader->update(); // schedules new background decodes
        g_loader->drainToTextures(g_maxUploadsPerFrame); // uploads a limited amount per frame
    }
}

void BaseRunner::render()
{
    this->window.clear();
    GameObjectManager::getInstance()->draw(&this->window);
    this->window.display();
}
