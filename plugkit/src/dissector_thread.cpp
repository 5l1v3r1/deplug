#include "dissector_thread.hpp"
#include "dissector.hpp"
#include "session_context.hpp"
#include "frame.hpp"
#include "layer.hpp"
#include "variant.hpp"
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <array>
#include <v8.h>

namespace plugkit {

class DissectorThread::Private {
public:
  FrameUniqueQueuePtr queue;
  Callback callback;
  std::vector<DissectorFactoryConstPtr> factories;
  std::unordered_map<Dissector::WorkerPtr, std::vector<std::regex>> workers;
  std::unordered_map<std::string, std::vector<Dissector::Worker *>> workerMap;
  Variant options;
};

DissectorThread::DissectorThread(const Variant &options,
                                 const FrameUniqueQueuePtr &queue,
                                 const Callback &callback)
    : d(new Private()) {
  d->options = options;
  d->queue = queue;
  d->callback = callback;
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
      d->workers[std::move(worker)] = diss->namespaces();
    }
  }
}

bool DissectorThread::loop() {
  std::array<FrameUniquePtr, 128> frames;
  size_t size = d->queue->dequeue(std::begin(frames), frames.size());
  if (size == 0)
    return false;

  for (size_t i = 0; i < size; ++i) {
    std::unordered_set<std::string> dissectedNamespaces;

    FrameUniquePtr &frame = frames[i];
    const auto &rootLayer = frame->rootLayer();
    if (!rootLayer)
      continue;

    std::vector<LayerPtr> layers = {rootLayer};
    while (!layers.empty()) {
      std::vector<LayerPtr> nextlayers;
      for (const auto &layer : layers) {
        const std::string &ns = layer->ns();
        dissectedNamespaces.insert(ns);

        std::vector<Dissector::Worker *> *workers;
        auto it = d->workerMap.find(ns);
        if (it != d->workerMap.end()) {
          workers = &it->second;
        } else {
          workers = &d->workerMap[ns];
          for (const auto &pair : d->workers) {
            for (const auto &regex : pair.second) {
              if (std::regex_search(ns, regex)) {
                workers->push_back(pair.first.get());
              }
            }
          }
        }
        std::vector<LayerPtr> childLayers;
        for (auto *worker : *workers) {
          if (const LayerPtr &childLayer = worker->analyze(layer)) {
            childLayer->setParent(layer);
            childLayers.push_back(childLayer);
            if (dissectedNamespaces.count(childLayer->ns()) == 0) {
              nextlayers.push_back(childLayer);
            }
          }
        }
        std::sort(childLayers.begin(), childLayers.end(),
                  [](const LayerConstPtr &a, const LayerConstPtr &b) {
                    return b->confidence() < a->confidence();
                  });
        for (const LayerPtr &child : childLayers) {
          layer->addChild(child);
        }
      }
      layers.swap(nextlayers);
    }
  }

  if (d->callback) {
    d->callback(&frames.front(), size);
  }
  /*
    using namespace v8;
    Isolate *isolate = Isolate::GetCurrent();
    Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::String> source =
        v8::String::NewFromUtf8(isolate, R"(
              var crypto = require('crypto');
              var shasum = crypto.createHash('sha1');
              shasum.update('my data');
              var hash = shasum.digest('hex');
              console.log(hash);
            )",
                                v8::NewStringType::kNormal)
            .ToLocalChecked();

    v8::Local<v8::Script> script =
        v8::Script::Compile(context, source).ToLocalChecked();
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

    v8::String::Utf8Value utf8(result);
    printf("%s\n", *utf8);
  */
  return true;
}

void DissectorThread::exit() {
  d->workers.clear();
  d->workerMap.clear();
}
}