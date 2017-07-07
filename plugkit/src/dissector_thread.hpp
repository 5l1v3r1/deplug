#ifndef PLUGKIT_DISSECTOR_THREAD_H
#define PLUGKIT_DISSECTOR_THREAD_H

#include "queue.hpp"
#include "worker_thread.hpp"
#include <memory>

namespace plugkit {

class Frame;
using FrameConstPtr = const Frame *;
using FrameUniquePtr = Frame *;

using FrameUniqueQueue = Queue<FrameUniquePtr>;
using FrameUniqueQueuePtr = std::shared_ptr<FrameUniqueQueue>;

class DissectorFactory;
using DissectorFactoryConstPtr = std::shared_ptr<const DissectorFactory>;

class Variant;

class DissectorThread final : public WorkerThread {
public:
  using Callback = std::function<void(FrameUniquePtr *, size_t)>;

public:
  DissectorThread(const Variant &options, const FrameUniqueQueuePtr &queue,
                  const Callback &callback);
  ~DissectorThread() override;
  void pushDissectorFactory(const DissectorFactoryConstPtr &factory);
  void enter() override;
  bool loop() override;
  void exit() override;
  uint32_t queue() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
