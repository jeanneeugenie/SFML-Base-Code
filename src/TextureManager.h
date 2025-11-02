#pragma once
#include <unordered_map>
#include "SFML/Graphics.hpp"

class TextureManager
{
public:
	typedef std::string String;
	typedef std::vector<sf::Texture*> TextureList;
	typedef std::unordered_map<String, TextureList> HashTable;
	// declaration for a new method.
	bool instantiateFromImage(const std::string& assetName,
		const sf::Image& img,
		bool isStreaming);
	
public:
	static TextureManager* getInstance();
	void loadFromAssetList(); //loading of all assets needed for startup
	void loadSingleStreamAsset(int index); //loads a single streaming asset based on index in directory
	sf::Texture* getFromTextureMap(const String assetName, int frameIndex);
	int getNumFrames(const String assetName);
	// Returns the current list of loaded streaming textures.
	// This allows progress tracking or debugging.
	const TextureList& getStreamingTextureList() const;

	sf::Texture* getStreamTextureFromList(const int index);
	int getNumLoadedStreamTextures() const;

private:
	TextureManager();
	TextureManager(TextureManager const&) {};             // copy constructor is private
	TextureManager& operator=(TextureManager const&) {};  // assignment operator is private
	static TextureManager* sharedInstance;
	int totalTilesLoaded; // [ADDED] counts how many textures (tiles) are loaded
	HashTable textureMap;
	TextureList baseTextureList;
	TextureList streamTextureList;

	const std::string STREAMING_PATH = "Media/Streaming/";
	int streamingAssetCount = 0;

	void countStreamingAssets();
	void instantiateAsTexture(String path, String assetName, bool isStreaming);

};