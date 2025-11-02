#include "BatchAssetLoader.h"
#include <filesystem>
#include <iostream>
#include <thread>
#include <random>

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
    const int currentBatch = nextBatchId_++;          // NEW: tag all items in this batch
    int end = std::min(submitted_ + n, (int)paths_.size());

    // Optional: visible console marker per batch
    std::cout << "=== Schedule Batch " << currentBatch
        << " (" << (end - submitted_) << " items) ===\n";

    // Per-file demo latency RNG (safe: worker threads only)
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> demoDelayMs(120, 240); // tweak if you want

    for (int i = submitted_; i < end; ++i) {
        std::string full = paths_[i];
        pool_.enqueue([this, full, currentBatch, demoDelay = demoDelayMs(rng)] {
            // --- demo-only delay; REMOVE for real benchmarks if desired ---
            std::this_thread::sleep_for(std::chrono::milliseconds(demoDelay));
            // ---------------------------------------------------------------

            sf::Image img;
            if (img.loadFromFile(full)) {
                DecodedImage di;
                di.fullPath = full;
                di.assetName = stem_from_filename(full);
                di.image = std::move(img);
                di.streaming = streaming_;
                di.batchId = currentBatch;           // NEW: preserve batch ID
                ready_.push(std::move(di));
                std::cout << "[Worker] Decoded " << di.assetName
                    << " (batch " << currentBatch << ")\n";
            }
            else {
                std::cerr << "[Worker] Failed to load " << full << "\n";
            }
            });
    }
    submitted_ = end;
}

/*
 * update
 * -------
 * Called every frame from the main thread. Never blocks.
 */
void BatchAssetLoader::update() {
    auto now = std::chrono::steady_clock::now();

    //removing sleep but having like a timer to make it work

    if (now - last_ >= interval_ && submitted_ < (int)paths_.size()) {
        schedule_batch(batchSize_);
        last_ = std::chrono::steady_clock::now();
    }

    // (Optional) debug:
     if (ready_.size() > 0)
         std::cout << "[BatchLoader] ready=" << ready_.size()
                   << " submitted=" << submitted_ << "/" << paths_.size() << "\n";
}

/*
 * drainToTextures
 * ----------------
 * Main-thread: upload limited number of textures to GPU per frame.
 * Notifies UI via onUpload_ with the batchId of each uploaded item.
 */
int BatchAssetLoader::drainToTextures(int maxUploadsPerFrame) {
    int uploaded = 0;

    while (uploaded < maxUploadsPerFrame) {
        auto maybe = ready_.try_pop();
        if (!maybe) break;

        sink_->createTextureFromImage(*maybe);

        if (onUpload_) onUpload_(maybe->batchId);   // NEW: notify UI/overlay

        ++uploaded;
        std::cout << "[Main] Uploaded: " << maybe->assetName
            << " (batch " << maybe->batchId << ")\n";
    }
    return uploaded;
}
