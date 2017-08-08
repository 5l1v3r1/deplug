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

const std::unordered_map<uint16_t, Token> protoTable = {
    {0x01, Token_get("[icmp]")},
    {0x02, Token_get("[igmp]")},
    {0x06, Token_get("[tcp]")},
    {0x11, Token_get("[udp]")},
};

const auto ipv6Token = Token_get("ipv6");
const auto versionToken = Token_get("version");
const auto tClassToken = Token_get("tClass");
const auto fLevelToken = Token_get("fLevel");
const auto pLenToken = Token_get("pLen");
const auto nHeaderToken = Token_get("nHeader");
const auto hLimitToken = Token_get("hLimit");
const auto srcToken = Token_get("src");
const auto dstToken = Token_get("dst");
const auto hbyhToken = Token_get("hbyh");
const auto protocolToken = Token_get("protocol");
const auto ipv6AddrToken = Token_get(":ipv6:addr");
const auto nestedToken = Token_get(":nested");

void analyze(Context *ctx, Worker *data, Layer *layer) {
  Reader reader;
  Reader_reset(&reader);
  reader.slice = Payload_data(Layer_payloads(layer, nullptr)[0]);

  Layer *child = Layer_addLayer(layer, ipv6Token);
  Layer_addTag(child, ipv6Token);

  uint8_t header = Reader_readUint8(&reader);
  uint8_t header2 = Reader_readUint8(&reader);
  int version = header >> 4;
  int trafficClass = (header & 0b00001111 << 4) | ((header2 & 0b11110000) >> 4);
  int flowLevel = Reader_readUint16BE(&reader) | ((header2 & 0b00001111) << 16);

  Property *ver = Layer_addProperty(child, versionToken);
  Variant_setUint64(Property_valueRef(ver), version);
  Property_setRange(ver, Range{0, 1});

  Property *tClass = Layer_addProperty(child, tClassToken);
  Variant_setUint64(Property_valueRef(tClass), trafficClass);
  Property_setRange(tClass, Range{0, 2});

  Property *fLevel = Layer_addProperty(child, fLevelToken);
  Variant_setUint64(Property_valueRef(fLevel), flowLevel);
  Property_setRange(fLevel, Range{1, 4});

  Property *pLen = Layer_addProperty(child, pLenToken);
  Variant_setUint64(Property_valueRef(pLen), Reader_readUint16BE(&reader));
  Property_setRange(pLen, reader.lastRange);

  int nextHeader = Reader_readUint8(&reader);
  auto nextHeaderRange = reader.lastRange;

  Property *nHeader = Layer_addProperty(child, nHeaderToken);
  Variant_setUint64(Property_valueRef(nHeader), nextHeader);
  Property_setRange(nHeader, nextHeaderRange);

  Property *hLimit = Layer_addProperty(child, hLimitToken);
  Variant_setUint64(Property_valueRef(hLimit), Reader_readUint8(&reader));
  Property_setRange(hLimit, reader.lastRange);

  const auto &srcSlice = Reader_slice(&reader, 0, 16);
  Property *src = Layer_addProperty(child, srcToken);
  Variant_setSlice(Property_valueRef(src), srcSlice);
  Property_setType(src, ipv6AddrToken);
  Property_setRange(src, reader.lastRange);

  const auto &dstSlice = Reader_slice(&reader, 0, 16);
  Property *dst = Layer_addProperty(child, dstToken);
  Variant_setSlice(Property_valueRef(dst), dstSlice);
  Property_setType(dst, ipv6AddrToken);
  Property_setRange(dst, reader.lastRange);

  bool ext = true;
  while (ext) {
    int header = 0;
    switch (nextHeader) {
    case 0:
    case 60: // Hop-by-Hop Options, Destination Options
    {
      header = Reader_readUint8(&reader);
      size_t extLen = Reader_readUint8(&reader);
      size_t byteLen = (extLen + 1) * 8;
      Reader_slice(&reader, 0, byteLen);
      Token id = (nextHeader == 0) ? hbyhToken : dstToken;
    }

    break;
    // TODO:
    // case 43  # Routing
    // case 44  # Fragment
    // case 51  # Authentication Header
    // case 50  # Encapsulating Security Payload
    // case 135 # Mobility
    case 59: // No Next Header
    default:
      ext = false;
      continue;
    }

    nextHeader = header;
  }

  uint8_t protocolNumber = nextHeader;
  Property *proto = Layer_addProperty(child, protocolToken);
  Variant_setUint64(Property_valueRef(proto), protocolNumber);
  Property_setType(proto, nestedToken);
  Property_setRange(proto, reader.lastRange);
  const auto &it = protoTable.find(protocolNumber);
  if (it != protoTable.end()) {
    Property *sub = Property_addProperty(proto, it->second);
    Variant_setBool(Property_valueRef(sub), true);
    Property_setRange(sub, reader.lastRange);
    Layer_addTag(child, it->second);
  }

  /*
        const std::string &summary =
            (src->summary() > dst->summary())
                ? src->summary() + " -> " + dst->summary()
                : dst->summary() + " <- " + src->summary();

        child->setSummary("[" + proto->summary() + "] " + summary);
        */
  Layer_addPayload(child, Reader_sliceAll(&reader, 0));
}
}

void Init(v8::Local<v8::Object> exports) {
  static Dissector diss;
  diss.layerHints[0] = Token_get("[ipv6]");
  diss.analyze = analyze;
  exports->Set(Nan::New("dissector").ToLocalChecked(),
               Nan::New<v8::External>(&diss));
}

NODE_MODULE(dissectorEssentials, Init);
