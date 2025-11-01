#include "TextureDisplay.h"
#include <iostream>
#include "TextureManager.h"
#include "BaseRunner.h"
#include "GameObjectManager.h"
#include "IconObject.h"
#include <filesystem>  // add this if not present

TextureDisplay::TextureDisplay(): AGameObject("TextureDisplay")
{
	
}

void TextureDisplay::initialize()
{
	
}

void TextureDisplay::processInput(sf::Event event)
{
	
}

void TextureDisplay::update(sf::Time deltaTime)
{
	this->ticks += BaseRunner::TIME_PER_FRAME.asMilliseconds();

	//<code here for spawning icon object periodically>
	 // Start streaming after a delay so the background/UI appear first
	if (!this->startedStreaming && this->ticks >= this->STREAMING_LOAD_DELAY) {
		this->startedStreaming = true;
		this->ticks = 0.0f; // reset if you want to pace spawning by time later
	}

	if (!this->startedStreaming) return;

	// Compute total streamable assets once
	static int totalStreamAssets = -1;
	if (totalStreamAssets < 0) {
		totalStreamAssets = 0;
		for (const auto& _ : std::filesystem::directory_iterator("Media/Streaming/")) {
			(void)_; // silence unused warning
			totalStreamAssets++;
		}
	}

	// Stream one icon per update (simple & smooth)
	int loaded = TextureManager::getInstance()->getNumLoadedStreamTextures();
	if (loaded < totalStreamAssets) {
		TextureManager::getInstance()->loadSingleStreamAsset(loaded);
		this->spawnObject();
	}

	ticks += deltaTime.asMilliseconds();

	/*if (ticks > STREAMING_LOAD_DELAY) {
		int texCount = TextureManager::getInstance()->getNumLoadedStreamTextures();

		if (texCount < 200) {
			LoadAssetThread* asset = new LoadAssetThread(TexCount, this);
			asset->start();
		}

		ticks = 0;
	}*/
}

void TextureDisplay::spawnObject()
{
	String objectName = "Icon_" + to_string(this->iconList.size());
	IconObject* iconObj = new IconObject(objectName, this->iconList.size());
	this->iconList.push_back(iconObj);

	//set position
	int IMG_WIDTH = 68; int IMG_HEIGHT = 68;
	float x = this->columnGrid * IMG_WIDTH;
	float y = this->rowGrid * IMG_HEIGHT;
	iconObj->setPosition(x, y);

	std::cout << "Set position: " << x << " " << y << std::endl;

	this->columnGrid++;
	if(this->columnGrid == this->MAX_COLUMN)
	{
		this->columnGrid = 0;
		this->rowGrid++;
	}
	GameObjectManager::getInstance()->addObject(iconObj);
}
