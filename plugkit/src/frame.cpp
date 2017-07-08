#include "private/frame.hpp"
#include "wrapper/frame.hpp"

namespace plugkit {

Frame *Frame::Private::create() {
  // TODO:ALLOC
  return new Frame();
}

Frame::Frame() : d(new Private()) {}

Frame::~Frame() {}

Frame::Timestamp Frame::timestamp() const { return d->timestamp(); }

Frame::Timestamp Frame::Private::timestamp() const { return timestamp_; }

void Frame::Private::setTimestamp(const Timestamp &timestamp) {
  timestamp_ = timestamp;
}

size_t Frame::length() const { return d->length(); }

size_t Frame::Private::length() const { return length_; }

void Frame::Private::setLength(size_t length) { length_ = length; }

uint32_t Frame::index() const { return d->index(); }

uint32_t Frame::Private::index() const { return seq_; }

void Frame::Private::setIndex(uint32_t index) { seq_ = index; }

Layer *Frame::rootLayer() const { return d->rootLayer(); }

Layer *Frame::Private::rootLayer() const { return layer_; }

void Frame::Private::setRootLayer(Layer *layer) { layer_ = layer; }

uint32_t Frame::sourceId() const { return d->sourceId(); }

uint32_t Frame::Private::sourceId() const { return sourceId_; }

void Frame::Private::setSourceId(uint32_t id) { sourceId_ = id; }
}
