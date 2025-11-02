#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include "ThreadPool.h"
#include "ConcurrentQueue.h"

/*
 * DecodedImage
 * -------------
 * Image decoded on a worker thread. batchId marks which batch scheduled it.
 */
struct DecodedImage {
    std::string assetName;
    std::string fullPath;
    sf::Image   image;
    bool        streaming = false;
    int         batchId = -1;     // NEW: which batch produced this item
};

class ITextureSink {
public:
    virtual ~ITextureSink() = default;
    virtual void createTextureFromImage(const DecodedImage& di) = 0; // main thread
};

class BatchAssetLoader {
public:
    BatchAssetLoader(ITextureSink* sink,
        std::vector<std::string> paths,
        int batchSize,
        int intervalMs,
        int workerThreads = 4,
        bool streaming = true);

    void update();                                   // call every frame
    int  drainToTextures(int maxUploadsPerFrame);    // main thread

    // Optional: observer for UI or logging when uploads happen
    void setOnUpload(std::function<void(int batchId)> cb) { onUpload_ = std::move(cb); }

private:
    void schedule_batch(int n);

private:
    ITextureSink* sink_;
    std::vector<std::string>      paths_;
    const int                     batchSize_;
    const std::chrono::milliseconds interval_;
    ThreadPool                    pool_;
    ConcurrentQueue<DecodedImage> ready_;
    std::chrono::steady_clock::time_point last_;
    int                           submitted_ = 0;
    bool                          streaming_ = true;

    // NEW: batch bookkeeping
    int                           nextBatchId_ = 0;
    std::function<void(int)>      onUpload_; // called per uploaded item (passes batchId)
};
