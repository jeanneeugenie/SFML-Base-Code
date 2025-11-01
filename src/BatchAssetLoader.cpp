#include "BatchAssetLoader.h"
#include <filesystem>

/*
 * Utility: extract filename without path or extension
 * just makes the printing part in cmdprompt neater
 */
static std::string stem_from_filename(const std::string& filename) {
    auto p = filename.find_last_of("/\\");
    std::string base = (p == std::string::npos) ? filename : filename.substr(p + 1);
    auto dot = base.find_last_of('.');
    return (dot == std::string::npos) ? base : base.substr(0, dot);
}

/*
 * Constructor - Stores parameters and initializes timers.
 */
BatchAssetLoader::BatchAssetLoader(ITextureSink* sink,
    std::vector<std::string> paths,
    int batchSize,
    int intervalMs,
    int workerThreads,
    bool streaming)
    : sink_(sink),
    paths_(std::move(paths)),
    batchSize_(batchSize),
    interval_(std::chrono::milliseconds(intervalMs)),
    pool_(workerThreads),
    last_(std::chrono::steady_clock::now()),
    streaming_(streaming) {
}

/*
 * Submits up to N new jobs to the thread pool.
 * Each job decodes one image using sf::Image (CPU side).
 */
void BatchAssetLoader::schedule_batch(int n) {
    int end = std::min(submitted_ + n, (int)paths_.size());
    for (int i = submitted_; i < end; ++i) {
        std::string full = paths_[i];
        pool_.enqueue([this, full] {
            sf::Image img;
            if (img.loadFromFile(full)) {
                DecodedImage di;
                di.fullPath = full;
                di.assetName = stem_from_filename(full);
                di.image = std::move(img);
                di.streaming = streaming_;
                ready_.push(std::move(di));
            }
            });
    }
    submitted_ = end;
}

/*
 * update
 * -------
 * Called every frame.
 * If enough time has passed, schedules the next batch.
 */
void BatchAssetLoader::update() {
    auto now = std::chrono::steady_clock::now();
    if (now - last_ >= interval_) {
        last_ = now;
        schedule_batch(batchSize_);
    }
}

/*
 * Called on the main thread.
 * Takes up to 'maxUploadsPerFrame' decoded images from the queue,
 * and asks the sink to create textures (GPU uploads).
 * This might be the broken one
 */
int BatchAssetLoader::drainToTextures(int maxUploadsPerFrame) {
    int uploaded = 0;
    while (uploaded < maxUploadsPerFrame) {
        auto maybe = ready_.try_pop();
        if (!maybe) break;
        sink_->createTextureFromImage(*maybe);
        ++uploaded;
    }
    return uploaded;
}
