#include <fstream>
#include <iostream>
#include <filesystem>
#include "TextureManager.h"
#include "StringUtils.h"
#include "IETThread.h"

//a singleton class
TextureManager* TextureManager::sharedInstance = NULL;

TextureManager* TextureManager::getInstance() {
	if (sharedInstance == NULL) {
		//initialize
		sharedInstance = new TextureManager();
	}

	return sharedInstance;
}

TextureManager::TextureManager()
{
	this->countStreamingAssets();
}

void TextureManager::loadFromAssetList()
{
	std::cout << "[TextureManager] Reading from asset list" << std::endl;
	std::ifstream stream("Media/assets.txt");
	String path;

	while(std::getline(stream, path))
	{
		std::vector<String> tokens = StringUtils::split(path, '/');
		String assetName = StringUtils::split(tokens[tokens.size() - 1], '.')[0];
		this->instantiateAsTexture(path, assetName, false);
		std::cout << "[TextureManager] Loaded texture: " << assetName << std::endl;
	}
}

void TextureManager::loadSingleStreamAsset(int index)
{
	int fileNum = 0;
	
	for (const auto& entry : std::filesystem::directory_iterator(STREAMING_PATH)) {
		if(index == fileNum)
		{
			//simulate loading of very large file
			//<code here for thread sleeping. Fill this up only when instructor told so.>
			//IETThread::sleep(200);			
			/*<code here for loading asset>
			String path = entry.path().string();
			//String filename = entry.path().filename().string();

			//// remove extension to form a clean asset name
			//auto parts = StringUtils::split(filename, '.');
			//String assetName = parts.size() > 0 ? parts[0] : filename;*/

			auto filePath = entry.path();
			std::cout << filePath.filename() << std::endl;
			
			// register texture into streaming list + map
			this->instantiateAsTexture(filePath.string(), filePath.filename().string(), true);
	
			std::cout << "[TextureManager] Loaded streaming texture: " << filePath.filename().string() << std::endl;
			break;
		}

		fileNum++;
	}
}

sf::Texture* TextureManager::getFromTextureMap(const String assetName, int frameIndex)
{
	if (!this->textureMap[assetName].empty()) {
		return this->textureMap[assetName][frameIndex];
	}
	else {
		std::cout << "[TextureManager] No texture found for " << assetName << std::endl;
		return NULL;
	}
}

int TextureManager::getNumFrames(const String assetName)
{
	if (!this->textureMap[assetName].empty()) {
		return this->textureMap[assetName].size();
	}
	else {
		std::cout << "[TextureManager] No texture found for " << assetName << std::endl;
		return 0;
	}
}

sf::Texture* TextureManager::getStreamTextureFromList(const int index)
{
	return this->streamTextureList[index];
}

int TextureManager::getNumLoadedStreamTextures() const
{
	return this->streamTextureList.size();
}

void TextureManager::countStreamingAssets()
{
	this->streamingAssetCount = 0;
	for (const auto& entry : std::filesystem::directory_iterator(STREAMING_PATH)) {
		this->streamingAssetCount++;
	}
	std::cout << "[TextureManager] Number of streaming assets: " << this->streamingAssetCount << std::endl;
}

void TextureManager::instantiateAsTexture(String path, String assetName, bool isStreaming)
{
	auto* texture = new sf::Texture();
	if (!texture->loadFromFile(path)) {
		std::cerr << "[TextureManager] ERROR loading " << path << "\n";
		delete texture;
		return;
	}
	this->textureMap[assetName].push_back(texture);

	if (isStreaming) this->streamTextureList.push_back(texture);
	else             this->baseTextureList.push_back(texture);
}

/*
 * Called by TextureSink on the main thread to create a GPU texture
 * from an sf::Image decoded on a background thread.
 */
bool TextureManager::instantiateFromImage(const std::string& assetName,
	const sf::Image& img,
	bool isStreaming) {
	try {
		auto tex = new sf::Texture();
		if (!tex->loadFromImage(img)) {
			std::cerr << "[TextureManager] Failed GPU upload for " << assetName << "\n";
			delete tex;
			return false;
		}

		TextureList& bucket = isStreaming ? this->streamTextureList : this->baseTextureList;
		this->textureMap[assetName].push_back(tex);
		bucket.push_back(tex);
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "[TextureManager] Exception uploading " << assetName
			<< ": " << e.what() << "\n";
		return false;
	}
}

/*
 * getStreamingTextureList
 * -----------------------
 * Returns a reference to the vector that stores currently loaded streaming textures.
 * This allows the main loop (BaseRunner) to monitor how many have been uploaded so far.
 */
const TextureManager::TextureList& TextureManager::getStreamingTextureList() const {
	return this->streamTextureList;
}