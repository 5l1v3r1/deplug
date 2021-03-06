#include "stream_dissector_thread_pool.hpp"
#include "dissector.h"
#include "frame.hpp"
#include "frame_store.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "stream_dissector_thread.hpp"
#include "stream_logger.hpp"
#include "variant.hpp"
#include <array>
#include <thread>
#include <uv.h>

namespace plugkit {

class StreamDissectorThreadPool::Private {
public:
  Private(const Variant &options, const FrameStorePtr &store,
          const Callback &callback);
  ~Private();
  uint32_t updateIndex(int thread, uint32_t pushed = 0, uint32_t dissected = 0);

public:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  std::vector<std::unique_ptr<StreamDissectorThread>> threads;
  std::vector<Dissector> dissectors;
  std::thread thread;
  std::vector<std::pair<uint32_t, uint32_t>> threadStats;
  std::mutex mutex;
  const Variant options;
  const FrameStorePtr store;
  const Callback callback;
};

StreamDissectorThreadPool::Private::Private(const Variant &options,
                                            const FrameStorePtr &store,
                                            const Callback &callback)
    : options(options), store(store), callback(callback) {}

StreamDissectorThreadPool::Private::~Private() {}

uint32_t StreamDissectorThreadPool::Private::updateIndex(int thread,
                                                         uint32_t pushed,
                                                         uint32_t dissected) {
  std::lock_guard<std::mutex> lock(mutex);

  uint32_t min = 0;
  if (threadStats[thread].first < pushed) {
    threadStats[thread].first = pushed;
  }
  if (threadStats[thread].second < dissected) {
    threadStats[thread].second = dissected;
  }

  for (const auto &pair : threadStats) {
    if (min < pair.first) {
      min = pair.first;
    }
  }

  for (const auto &pair : threadStats) {
    if (pair.first > pair.second) {
      if (min > pair.second) {
        min = pair.second;
      }
    }
  }

  return min;
}

StreamDissectorThreadPool::StreamDissectorThreadPool(const Variant &options,
                                                     const FrameStorePtr &store,
                                                     const Callback &callback)
    : d(new Private(options, store, callback)) {}

StreamDissectorThreadPool::~StreamDissectorThreadPool() {
  for (const auto &thread : d->threads) {
    thread->stop();
  }
  for (const auto &thread : d->threads) {
    thread->join();
  }
  d->store->close();
  if (d->thread.joinable())
    d->thread.join();
}

void StreamDissectorThreadPool::registerDissector(const Dissector &diss) {
  d->dissectors.push_back(diss);
}

void StreamDissectorThreadPool::setLogger(const LoggerPtr &logger) {
  d->logger = logger;
}

void StreamDissectorThreadPool::start() {
  if (d->thread.joinable() || !d->threads.empty())
    return;

  int concurrency = d->options["_"]["concurrency"].uint64Value(0);
  if (concurrency == 0)
    concurrency = std::thread::hardware_concurrency();
  if (concurrency == 0)
    concurrency = 1;

  for (int i = 0; i < concurrency; ++i) {
    auto dissectorThread = new StreamDissectorThread(
        d->options, [this, i](uint32_t maxFrameIndex) {
          uint32_t index = d->updateIndex(i, 0, maxFrameIndex);
          d->callback(index);
        });
    for (const auto &diss : d->dissectors) {
      dissectorThread->pushStreamDissector(diss);
    }
    dissectorThread->setLogger(d->logger);
    d->threads.emplace_back(dissectorThread);
  }

  d->threadStats.resize(d->threads.size());

  for (const auto &thread : d->threads) {
    thread->start();
  }

  d->thread = std::thread([this, concurrency]() {
    size_t offset = 0;
    std::array<const Frame *, 128> frames;
    while (true) {
      size_t size = d->store->dequeue(offset, frames.size(), &frames.front());
      if (size == 0)
        return;
      offset += size;

      std::function<std::vector<Layer *>(Layer *)> findStreamedLayers =
          [&findStreamedLayers](Layer *layer) -> std::vector<Layer *> {

        std::vector<Layer *> layers;
        if (layer->layers().empty()) {
          layers.push_back(layer);
        } else {
          for (const auto &child : layer->layers()) {
            const auto &childList = findStreamedLayers(child);
            layers.insert(layers.begin(), childList.begin(), childList.end());
          }
        }
        return layers;
      };

      std::vector<std::vector<Layer *>> layerMap;
      layerMap.resize(concurrency);
      for (size_t i = 0; i < size; ++i) {
        const auto &layers = findStreamedLayers(frames[i]->rootLayer());
        for (Layer *layer : layers) {
          int thread = layer->worker() % d->threads.size();
          layerMap[thread].push_back(layer);
          if (const Frame *frame = layer->frame()) {
            d->updateIndex(thread, frame->index(), 0);
          }
        }
      }
      for (size_t i = 0; i < layerMap.size(); ++i) {
        auto &layer = layerMap[i];
        if (!layer.empty()) {
          d->threads[i]->push(&layer.front(), layer.size());
        }
      }
    }
  });
}
} // namespace plugkit
