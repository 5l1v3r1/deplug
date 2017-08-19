#include <nan.h>
#include <plugkit/dissector.h>
#include <plugkit/context.h>
#include <plugkit/token.h>
#include <plugkit/property.h>
#include <plugkit/variant.h>
#include <plugkit/layer.h>
#include <plugkit/payload.h>
#include <plugkit/reader.h>
#include <unordered_map>

using namespace plugkit;

const auto udpToken = Token_get("udp");
const auto srcToken = Token_get(".src");
const auto dstToken = Token_get(".dst");
const auto lengthToken = Token_get("udp.length");
const auto checksumToken = Token_get("udp.checksum");

namespace {
void analyze(Context *ctx, void *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.slice = Payload_data(Layer_payload(layer));

  Layer *child = Layer_addLayer(layer, udpToken);
  Layer_addTag(child, udpToken);

  const auto &parentSrc = Layer_propertyFromId(layer, srcToken);
  const auto &parentDst = Layer_propertyFromId(layer, dstToken);

  uint16_t sourcePort = Reader_readUint16BE(&reader);
  Property *src = Layer_addProperty(child, srcToken);
  Property_setUint64(src, sourcePort);
  Property_setRange(src, reader.lastRange);

  uint16_t dstPort = Reader_readUint16BE(&reader);
  Property *dst = Layer_addProperty(child, dstToken);
  Property_setUint64(dst, dstPort);
  Property_setRange(dst, reader.lastRange);

  uint32_t lengthNumber = Reader_readUint16BE(&reader);
  Property *length = Layer_addProperty(child, lengthToken);
  Property_setUint64(length, lengthNumber);
  Property_setRange(length, reader.lastRange);

  uint32_t checksumNumber = Reader_readUint16BE(&reader);
  Property *checksum = Layer_addProperty(child, checksumToken);
  Property_setUint64(checksum, checksumNumber);
  Property_setRange(checksum, reader.lastRange);

  Layer_addPayload(child, Reader_slice(&reader, 0, lengthNumber - 8));
}
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_PACKET);
  Dissector_addLayerHint(diss, Token_get("[udp]"));
  Dissector_setAnalyzer(diss, analyze);
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}

NODE_MODULE(dissectorEssentials, Init);
