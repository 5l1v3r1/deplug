#include "plugkit_module.hpp"
#include "extended_slot.hpp"
#include "wrapper/property.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/chunk.hpp"
#include "wrapper/session.hpp"
#include "wrapper/pcap.hpp"
#include "wrapper/dissector_factory.hpp"
#include "wrapper/stream_dissector_factory.hpp"
#include "wrapper/session_factory.hpp"

namespace plugkit {

PlugkitModule::PlugkitModule(v8::Isolate *isolate,
                             v8::Local<v8::Object> exports, bool mainThread) {
  ExtendedSlot::set(isolate, ExtendedSlot::SLOT_PLUGKIT_MODULE, this);
  PropertyWrapper::init(isolate, exports);
  LayerWrapper::init(isolate, exports);
  FrameWrapper::init(isolate);
  ChunkWrapper::init(isolate, exports);
  if (mainThread) {
    PcapWrapper::init(isolate, exports);
    SessionFactoryWrapper::init(isolate, exports);
    SessionWrapper::init(isolate, exports);
    DissectorFactoryWrapper::init(isolate);
    StreamDissectorFactoryWrapper::init(isolate);
  }
}
}
