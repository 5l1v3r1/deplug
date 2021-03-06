#ifndef PLUGKIT_WORKER_THREAD_H
#define PLUGKIT_WORKER_THREAD_H

#include "stream_logger.hpp"
#include <thread>

namespace plugkit {

class WorkerThread {
public:
  WorkerThread();
  virtual ~WorkerThread();
  WorkerThread(const WorkerThread &) = delete;
  WorkerThread &operator=(const WorkerThread &) = delete;
  virtual void enter() = 0;
  virtual bool loop() = 0;
  virtual void exit() = 0;
  void start();
  void join();
  void setLogger(const LoggerPtr &logger);

protected:
  LoggerPtr logger = std::make_shared<StreamLogger>();
  std::thread thread;

private:
  class ArrayBufferAllocator;
};
} // namespace plugkit

#endif
