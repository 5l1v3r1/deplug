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

namespace {

const std::pair<uint16_t, Token> flagTable[] = {
    std::make_pair(0x1, Token_get("ipv4.flags.reserved")),
    std::make_pair(0x2, Token_get("ipv4.flags.dontFrag")),
    std::make_pair(0x4, Token_get("ipv4.flags.moreFrag")),
};

const std::unordered_map<uint16_t, std::pair<Token, Token>> protoTable = {
    {0x01,
     std::make_pair(Token_get("[icmp]"), Token_get("ipv4.protocol.icmp"))},
    {0x02,
     std::make_pair(Token_get("[igmp]"), Token_get("ipv4.protocol.igmp"))},
    {0x06, std::make_pair(Token_get("[tcp]"), Token_get("ipv4.protocol.tcp"))},
    {0x11, std::make_pair(Token_get("[udp]"), Token_get("ipv4.protocol.udp"))},
};

const auto ipv4Token = Token_get("ipv4");
const auto versionToken = Token_get("ipv4.version");
const auto hLenToken = Token_get("ipv4.hLen");
const auto typeToken = Token_get("ipv4.type");
const auto tLenToken = Token_get("ipv4.tLen");
const auto idToken = Token_get("ipv4.id");
const auto flagsToken = Token_get("ipv4.flags");
const auto fOffsetToken = Token_get("ipv4.fOffset");
const auto ttlToken = Token_get("ipv4.ttl");
const auto protocolToken = Token_get("ipv4.protocol");
const auto checksumToken = Token_get("ipv4.checksum");
const auto srcToken = Token_get("ipv4.src");
const auto dstToken = Token_get("ipv4.dst");
const auto ipv4AddrToken = Token_get("@ipv4:addr");
const auto nestedToken = Token_get("@flags");

void analyze(Context *ctx, Worker *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.slice = Payload_data(Layer_payloads(layer, nullptr)[0]);

  Layer *child = Layer_addLayer(layer, ipv4Token);
  Layer_addTag(child, ipv4Token);

  uint8_t header = Reader_readUint8(&reader);
  int version = header >> 4;
  int headerLength = header & 0b00001111;

  Property *ver = Layer_addProperty(child, versionToken);
  Variant_setUint64(Property_valueRef(ver), version);
  Property_setRange(ver, reader.lastRange);

  Property *hlen = Layer_addProperty(child, hLenToken);
  Variant_setUint64(Property_valueRef(hlen), headerLength);
  Property_setRange(hlen, reader.lastRange);

  Property *tos = Layer_addProperty(child, typeToken);
  Variant_setUint64(Property_valueRef(tos), Reader_readUint8(&reader));
  Property_setRange(tos, reader.lastRange);

  uint16_t totalLength = Reader_readUint16BE(&reader);
  Property *tlen = Layer_addProperty(child, tLenToken);
  Variant_setUint64(Property_valueRef(tlen), totalLength);
  Property_setRange(tlen, reader.lastRange);

  Property *id = Layer_addProperty(child, idToken);
  Variant_setUint64(Property_valueRef(id), Reader_readUint16BE(&reader));
  Property_setRange(id, reader.lastRange);

  uint8_t flagAndOffset = Reader_readUint8(&reader);
  uint8_t flag = (flagAndOffset >> 5) & 0b00000111;

  Property *flags = Layer_addProperty(child, flagsToken);
  Variant_setUint64(Property_valueRef(flags), flag);
  std::string flagSummary;
  for (const auto &bit : flagTable) {
    bool on = bit.first & flag;
    Property *flagBit = Layer_addProperty(child, bit.second);
    Variant_setBool(Property_valueRef(flagBit), on);
    Property_setRange(flagBit, reader.lastRange);

    if (on) {
      if (!flagSummary.empty())
        flagSummary += ", ";
      flagSummary += std::get<1>(bit);
    }
  }
  Property_setType(flags, nestedToken);
  Property_setRange(flags, reader.lastRange);

  uint16_t fgOffset =
      ((flagAndOffset & 0b00011111) << 8) | Reader_readUint8(&reader);
  Property *fragmentOffset = Layer_addProperty(child, fOffsetToken);
  Variant_setUint64(Property_valueRef(fragmentOffset), fgOffset);
  Property_setRange(fragmentOffset, Range{6, 8});

  Property *ttl = Layer_addProperty(child, ttlToken);
  Variant_setUint64(Property_valueRef(ttl), Reader_readUint8(&reader));
  Property_setRange(ttl, reader.lastRange);

  uint8_t protocolNumber = Reader_readUint8(&reader);
  Property *proto = Layer_addProperty(child, protocolToken);
  Variant_setUint64(Property_valueRef(proto), protocolNumber);
  Property_setType(proto, nestedToken);
  Property_setRange(proto, reader.lastRange);
  const auto &it = protoTable.find(protocolNumber);
  if (it != protoTable.end()) {
    Property *sub = Layer_addProperty(child, it->second.second);
    Variant_setBool(Property_valueRef(sub), true);
    Property_setRange(sub, reader.lastRange);
    Layer_addTag(child, it->second.first);
  }

  Property *checksum = Layer_addProperty(child, checksumToken);
  Variant_setUint64(Property_valueRef(checksum), Reader_readUint16BE(&reader));
  Property_setRange(checksum, reader.lastRange);

  const auto &srcSlice = Reader_slice(&reader, 0, 4);
  Property *src = Layer_addProperty(child, srcToken);
  Variant_setSlice(Property_valueRef(src), srcSlice);
  Property_setType(src, ipv4AddrToken);
  Property_setRange(src, reader.lastRange);

  const auto &dstSlice = Reader_slice(&reader, 0, 4);
  Property *dst = Layer_addProperty(child, dstToken);
  Variant_setSlice(Property_valueRef(dst), dstSlice);
  Property_setType(dst, ipv4AddrToken);
  Property_setRange(dst, reader.lastRange);

  Layer_addPayload(child, Reader_sliceAll(&reader, 0));
}
}

void Init(v8::Local<v8::Object> exports) {
  Dissector *diss = Dissector_create(DISSECTOR_PACKET);
  Dissector_addLayerHint(diss, Token_get("[ipv4]"));
  Dissector_setAnalyzer(diss, analyze);
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(diss));
}

NODE_MODULE(dissectorEssentials, Init);
