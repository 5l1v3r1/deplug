#ifndef PLUGKIT_FRAME_VIEW_H
#define PLUGKIT_FRAME_VIEW_H

#include <memory>
#include <vector>
#include "strid.hpp"

namespace plugkit {

class Frame;
class FrameView;
class Layer;
class Property;

class FrameView final {
public:
  explicit FrameView(const Frame *frame);
  ~FrameView();
  const Frame *frame() const;
  const Layer *primaryLayer() const;
  const std::vector<const Layer *> &leafLayers() const;
  const Property *propertyFromId(strid id) const;
  const Layer *layerFromId(strid id) const;
  bool hasError() const;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
