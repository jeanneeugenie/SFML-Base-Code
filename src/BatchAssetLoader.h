#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <chrono>
#include "ThreadPool.h"
#include "ConcurrentQueue.h"
#include <SFML/System/Clock.hpp>

/*
 * Represents an image fully loaded and decoded in memory (CPU side).
 * basically a tile asset is placed into memory
 */
struct DecodedImage {
    std::string assetName;
    std::string fullPath;
    sf::Image image;
    bool streaming = false;
};

/*
 * Anything implementing this can receive decoded images and
 * convert them to GPU textures on the main thread.
 */
class ITextureSink {
public:
    virtual ~ITextureSink() = default;
    virtual void createTextureFromImage(const DecodedImage& di) = 0;
};

/*
 * BatchAssetLoader
 * ----------------
 * Handles:
 *   • Scheduling batches of file loads (background threads)
 *   • Timing the batch intervals
 *   • Delivering decoded images to the main thread in small chunks
 */
class BatchAssetLoader {
public:
    BatchAssetLoader(ITextureSink* sink,
        std::vector<std::string> paths,
        int batchSize,
        int intervalMs, 
        int workerThreads = 4,
        bool streaming = true);

    void update();                      // schedules next batch when interval passes
    int drainToTextures(int maxUploadsPerFrame);  // uploads limited textures per frame
    bool done() const { return submitted_ >= (int)paths_.size() && ready_.size() == 0; }

private:
    void schedule_batch(int n);

    ITextureSink* sink_;
    sf::Clock clock_;
    std::vector<std::string> paths_;
    const int batchSize_;
    const std::chrono::milliseconds interval_;
    ThreadPool pool_;
    ConcurrentQueue<DecodedImage> ready_;
    std::chrono::steady_clock::time_point last_;
    int submitted_ = 0;
    bool streaming_ = true;
};
