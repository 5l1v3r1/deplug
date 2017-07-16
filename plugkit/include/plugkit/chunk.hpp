#ifndef PLUGKIT_CHUNK_H
#define PLUGKIT_CHUNK_H

#include "export.hpp"
#include "types.hpp"
#include <memory>

namespace plugkit {

class Layer;

class PLUGKIT_EXPORT Chunk final {
public:
  Chunk(const Layer *layer, Range range = Range());
  ~Chunk();
  const Layer *layer() const;
  void setLayer(const Layer *layer);
  Range range() const;
  void setRange(const Range &range);

private:
  Chunk(const Chunk &chunk) = delete;
  Chunk &operator=(const Chunk &chunk) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
