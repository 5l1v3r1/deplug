#include "dissector_thread.hpp"
#include "dissector.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "session_context.hpp"
#include "variant.hpp"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <v8.h>

namespace plugkit {

namespace {
struct WorkerData {
  Dissector::WorkerPtr worker;
  std::vector<minins> namespaces;
  size_t streamIdLength = 0;
};
}

class DissectorThread::Private {
public:
  FrameQueuePtr queue;
  Callback callback;
  std::vector<DissectorFactoryConstPtr> factories;
  std::vector<WorkerData> workers;
  Variant options;
  double confidenceThreshold;
};

DissectorThread::DissectorThread(const Variant &options,
                                 const FrameQueuePtr &queue,
                                 const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->queue = queue;
  d->callback = callback;
  d->confidenceThreshold =
      options["_"]["confidenceThreshold"].uint64Value(0) / 100.0;
}

DissectorThread::~DissectorThread() {}

void DissectorThread::pushDissectorFactory(
    const DissectorFactoryConstPtr &factory) {
  d->factories.push_back(factory);
}

void DissectorThread::enter() {
  SessionContext ctx;
  ctx.logger = logger;
  ctx.options = d->options;
  for (const auto &factory : d->factories) {
    auto diss = factory->create(ctx);
    if (auto worker = diss->createWorker()) {
      WorkerData data;
      data.worker = std::move(worker);
      data.namespaces = diss->namespaces();
      data.streamIdLength = diss->streamIdLength();
      d->workers.emplace_back(std::move(data));
    }
  }
}

bool DissectorThread::loop() {
  std::array<Frame *, 128> frames;
  size_t size = d->queue->dequeue(std::begin(frames), frames.size());
  if (size == 0)
    return false;

  StreamIdMap streamIdMap;

  for (size_t i = 0; i < size; ++i) {
    std::unordered_set<minins> dissectedNamespaces;

    Frame *frame = frames[i];
    const auto &rootLayer = frame->rootLayer();
    if (!rootLayer)
      continue;

    std::vector<Dissector::Worker *> workers;
    std::vector<Layer *> leafLayers = {rootLayer};
    while (!leafLayers.empty()) {
      std::vector<Layer *> nextlayers;
      for (const auto &layer : leafLayers) {
        const minins &ns = layer->ns();
        dissectedNamespaces.insert(ns);

        workers.clear();
        for (const auto &data : d->workers) {
          for (const auto &filter : data.namespaces) {
            if (ns.match(filter)) {
              workers.push_back(data.worker.get());
            }
          }
        }

        std::vector<Layer *> childLayers;
        for (auto *worker : workers) {
          if (Layer *childLayer = worker->analyze(layer)) {
            if (childLayer->confidence() >= d->confidenceThreshold) {
              childLayer->setParent(layer);
              childLayer->setFrame(frame);
              childLayers.push_back(childLayer);
              if (dissectedNamespaces.count(childLayer->ns()) == 0) {
                nextlayers.push_back(childLayer);
              }
            }
          }
        }
        std::sort(childLayers.begin(), childLayers.end(),
                  [](const Layer *a, const Layer *b) {
                    return b->confidence() < a->confidence();
                  });
        for (Layer *child : childLayers) {
          layer->addChild(child);
        }
      }
      leafLayers.swap(nextlayers);
    }
  }

  if (d->callback) {
    d->callback(&frames.front(), size);
  }

  return true;
}

void DissectorThread::exit() { d->workers.clear(); }
}
