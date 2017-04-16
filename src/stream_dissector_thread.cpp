#include "stream_dissector_thread.hpp"
#include "stream_dissector.hpp"
#include "session_context.hpp"
#include "private/frame.hpp"
#include "chunk.hpp"
#include "layer.hpp"
#include "queue.hpp"
#include "variant.hpp"
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <array>
#include <v8.h>

namespace plugkit {

class StreamDissectorThread::Private {
public:
  void cleanup();

public:
  Variant options;
  Callback callback;
  Queue<ChunkConstPtr> queue;
  std::vector<StreamDissectorFactoryConstPtr> factories;
  std::unordered_map<StreamDissectorPtr, std::vector<std::regex>> dissectors;
  std::unordered_map<std::string, std::vector<StreamDissector *>> dissectorMap;
  size_t count = 0;

  struct WorkerList {
    std::vector<StreamDissector::WorkerPtr> list;
    StreamDissector::Worker::Timestamp lastUpdated;
  };

  using IdMap = std::unordered_map<std::string, WorkerList>;
  std::unordered_map<std::string, IdMap> workers;
};

void StreamDissectorThread::Private::cleanup() {
  for (auto &pair : workers) {
    for (auto it = pair.second.begin(); it != pair.second.end();) {
      bool alive = false;
      for (auto &worker : it->second.list) {
        if (!worker->expired(it->second.lastUpdated)) {
          alive = true;
        }
      }
      if (alive) {
        ++it;
      } else {
        it = pair.second.erase(it);
      }
    }
  }
}

StreamDissectorThread::StreamDissectorThread(const Variant &options,
                                             const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->callback = callback;
}

StreamDissectorThread::~StreamDissectorThread() {}

void StreamDissectorThread::pushStreamDissectorFactory(
    const StreamDissectorFactoryConstPtr &factory) {
  d->factories.push_back(factory);
}

void StreamDissectorThread::enter() {
  SessionContext ctx;
  ctx.logger = logger;
  ctx.options = d->options;
  for (const auto &factory : d->factories) {
    StreamDissectorPtr diss = factory->create(ctx);
    const auto &namespaces = diss->namespaces();
    d->dissectors[std::move(diss)] = namespaces;
  }
}

bool StreamDissectorThread::loop() {
  std::array<ChunkConstPtr, 128> chunks;
  size_t size = d->queue.dequeue(std::begin(chunks), chunks.size());
  if (size == 0)
    return false;

  for (size_t i = 0; i < size; ++i) {
    const auto &chunk = chunks[i];
    const std::string &ns = chunk->streamNs();
    auto &workers = d->workers[ns][chunk->streamId()];

    if (workers.list.empty()) {
      std::vector<StreamDissector *> *dissectors;
      auto it = d->dissectorMap.find(ns);
      if (it != d->dissectorMap.end()) {
        dissectors = &it->second;
      } else {
        dissectors = &d->dissectorMap[ns];
        for (const auto &pair : d->dissectors) {
          for (const auto &regex : pair.second) {
            if (std::regex_match(ns, regex)) {
              dissectors->push_back(pair.first.get());
            }
          }
        }
      }

      for (auto dissector : *dissectors) {
        if (auto worker = dissector->createWorker()) {
          workers.list.push_back(std::move(worker));
        }
      }
    }

    std::vector<FrameUniquePtr> frames;
    for (const auto &worker : workers.list) {
      if (const auto &layer = worker->analyze(chunk)) {
        auto frame = Frame::Private::create();
        frame->d->setRootLayer(layer);
        frames.push_back(std::move(frame));
      }
    }

    if (!workers.list.empty()) {
      workers.lastUpdated = std::chrono::system_clock::now();
    }

    d->callback(&frames.front(), frames.size());
  }
  d->count++;
  if (d->count % 1024 == 0) {
    d->cleanup();
  }
  return true;
}

void StreamDissectorThread::exit() {
  d->factories.clear();
  d->dissectors.clear();
  d->workers.clear();
}

void StreamDissectorThread::push(const ChunkConstPtr *begin, size_t size) {
  d->queue.enqueue(begin, begin + size);
}

void StreamDissectorThread::stop() { d->queue.close(); }
}
