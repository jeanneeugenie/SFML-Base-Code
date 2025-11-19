#pragma once
#include "AGameObject.h"
#include "TextureManager.h"

#include "iostream"

class BatchAssetLoader;

class LoadingGif : public AGameObject {
public:
	std::string name;
	LoadingGif(std::string Name, BatchAssetLoader* loader = nullptr);

	void initialize() override;
	void processInput(sf::Event event) override;
	void update(sf::Time deltaTime) override;
	void draw(sf::RenderWindow* targetWindow) override; // new

	// allow assigning loader after construction
	void setLoader(BatchAssetLoader* loader) { loader_ = loader; }

private:
	BatchAssetLoader* loader_ = nullptr;
	float margin_ = 20.f;
};