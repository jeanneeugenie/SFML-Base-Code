#include "BaseRunner.h"
#include "GameObjectManager.h"
#include "BGObject.h"
#include "TextureManager.h"
#include "TextureDisplay.h"
#include "FPSCounter.h"

// New headers for batch loading
#include "BatchAssetLoader.h"
#include "TextureSink.h"
#include <filesystem>

const sf::Time BaseRunner::TIME_PER_FRAME = sf::seconds(1.f / 60.f);

// globals owned by BaseRunner
std::unique_ptr<BatchAssetLoader> g_loader;
TextureSink g_sink;

BaseRunner::BaseRunner() :
	window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
		"HO: Entity Component", sf::Style::Close)
{
	// 1) Load initial textures synchronously
	TextureManager::getInstance()->loadFromAssetList();

	// 2) Initialize scene objects
	BGObject* bgObject = new BGObject("BGObject");
	GameObjectManager::getInstance()->addObject(bgObject);

	TextureDisplay* display = new TextureDisplay();
	GameObjectManager::getInstance()->addObject(display);

	FPSCounter* fpsCounter = new FPSCounter();
	GameObjectManager::getInstance()->addObject(fpsCounter);

	// 3) Prepare list of streaming files (Media/Streaming)
	std::vector<std::string> streamingFiles;
	for (auto& entry : std::filesystem::directory_iterator("Media/Streaming"))
	{
		if (!entry.is_regular_file()) continue;
		auto path = entry.path().string();
		if (path.ends_with(".png") || path.ends_with(".jpg"))
			streamingFiles.push_back(path);
	}

	// 4) Initialize loader: loads 15 assets every 120 ms, up to 4 worker threads
	g_loader = std::make_unique<BatchAssetLoader>(
		&g_sink,
		streamingFiles,
		/*batchSize*/ 15,
		/*intervalMs*/ 120,
		/*workerThreads*/ 4,
		/*streaming*/ true
	);
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
	if (this->window.pollEvent(event))
	{
		switch (event.type)
		{
		default:
			GameObjectManager::getInstance()->processInput(event);
			break;
		case sf::Event::Closed:
			this->window.close();
			break;
		}
	}
}

void BaseRunner::update(sf::Time elapsedTime)
{
	GameObjectManager::getInstance()->update(elapsedTime);

	// run batch asset loader every frame (it internally throttles itself)
	if (g_loader)
	{
		g_loader->update();               // schedule background decode
		g_loader->drainToTextures(10);    // upload up to 10 textures/frame
	}
}

void BaseRunner::render()
{
	this->window.clear();
	GameObjectManager::getInstance()->draw(&this->window);
	this->window.display();
}
