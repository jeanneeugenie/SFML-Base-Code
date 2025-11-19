#include "LoadingGif.h"
#include "BatchAssetLoader.h"
#include "BaseRunner.h"

LoadingGif::LoadingGif(std::string Name, BatchAssetLoader* loader) : AGameObject(Name), loader_(loader)
{
	this->name = Name;
}


void LoadingGif::initialize()
{
	std::cout << "Declared as " << this->getName() << "\n";

	// assign texture
	this->sprite = new sf::Sprite();
	sf::Texture* texture = TextureManager::getInstance()->gifHolder[0];
	if (texture) this->sprite->setTexture(*texture);
	sf::Vector2u textureSize = this->sprite->getTexture()->getSize();
	// start at bottom-right by default
	this->setPosition(BaseRunner::WINDOW_WIDTH - textureSize.x - margin_, BaseRunner::WINDOW_HEIGHT - textureSize.y - margin_);

	// Render above overlay but below FPSCounter
	this->setZOrder(150);
}

void LoadingGif::processInput(sf::Event event)
{
}

void LoadingGif::update(sf::Time deltaTime)
{
	if (!loader_) return;
	// compute progress considering uploaded, decoded-ready and in-flight items
	int uploaded = loader_->getUploaded();
	int submitted = loader_->getSubmitted();
	auto ready = static_cast<int>(loader_->getReadyCount());
	int inflight = loader_->getInFlight();
	int total = loader_->getTotal();

	float pct = 1.0f;
	if (total > 0) {
		float effective = static_cast<float>(uploaded + ready + inflight);
		pct = effective / static_cast<float>(total);
	} else {
		pct = 1.0f;
	}

	if (pct < 0.f) pct = 0.f; if (pct > 1.f) pct = 1.f;

	// leftmost and rightmost available x positions
	sf::Vector2u texSize = this->sprite->getTexture()->getSize();
	float rightX = BaseRunner::WINDOW_WIDTH - texSize.x - margin_;
	float leftX = margin_;
	// position moves from right (0%) to left (100%) so invert pct
	float x = rightX + (leftX - rightX) * pct;
	float y = BaseRunner::WINDOW_HEIGHT - texSize.y - margin_;

	this->setPosition(x, y);
}

void LoadingGif::draw(sf::RenderWindow* targetWindow)
{
	// don't draw when loader finished (overlay removed)
	if (loader_ && loader_->isFinished()) return;

	// otherwise draw normally
	AGameObject::draw(targetWindow);
}
