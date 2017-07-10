#include <nan.h>
#include <plugkit/dissector.hpp>
#include <plugkit/stream_dissector.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/property.hpp>
#include <plugkit/chunk.hpp>
#include <plugkit/fmt.hpp>
#include <unordered_map>

using namespace plugkit;

class TCPDissector final : public Dissector {
public:
  class Worker final : public Dissector::Worker {
  public:
    Layer* analyze(const Layer*layer) override {
      fmt::Reader<Slice> reader(layer->payload());
      Layer child(PK_STRNS("tcp"));

      const auto& parentSrc = layer->propertyFromId(PK_STRID("src"));
      const auto& parentDst = layer->propertyFromId(PK_STRID("dst"));

      uint16_t sourcePort = reader.readBE<uint16_t>();
      Property src(PK_STRID("src"), sourcePort);
      src.setSummary(parentSrc->summary() + ":" + std::to_string(sourcePort));
      src.setRange(reader.lastRange());
      src.setError(reader.lastError());

      uint16_t dstPort = reader.readBE<uint16_t>();
      Property dst(PK_STRID("dst"), dstPort);
      dst.setSummary(parentDst->summary() + ":" + std::to_string(dstPort));
      dst.setRange(reader.lastRange());
      dst.setError(reader.lastError());

      child.setSummary(src.summary() + " -> " + dst.summary());

      uint32_t seqNumber = reader.readBE<uint32_t>();
      Property seq(PK_STRID("seq"), seqNumber);
      seq.setRange(reader.lastRange());
      seq.setError(reader.lastError());

      uint32_t ackNumber = reader.readBE<uint32_t>();
      Property ack(PK_STRID("ack"), ackNumber);
      ack.setRange(reader.lastRange());
      ack.setError(reader.lastError());

      uint8_t offsetAndFlag = reader.readBE<uint8_t>();
      int dataOffset = offsetAndFlag >> 4;
      Property offset(PK_STRID("dOffset"), dataOffset);
      offset.setRange(reader.lastRange());
      offset.setError(reader.lastError());

      uint8_t flag = reader.readBE<uint8_t>() |
        ((offsetAndFlag & 0x1) << 8);

      const std::tuple<uint16_t, const char*, strid> flagTable[] = {
        std::make_tuple(0x1 << 8, "NS",  PK_STRID("ns")  ),
        std::make_tuple(0x1 << 7, "CWR", PK_STRID("cwr") ),
        std::make_tuple(0x1 << 6, "ECE", PK_STRID("ece") ),
        std::make_tuple(0x1 << 5, "URG", PK_STRID("urg") ),
        std::make_tuple(0x1 << 4, "ACK", PK_STRID("ack") ),
        std::make_tuple(0x1 << 3, "PSH", PK_STRID("psh") ),
        std::make_tuple(0x1 << 2, "RST", PK_STRID("rst") ),
        std::make_tuple(0x1 << 1, "SYN", PK_STRID("syn") ),
        std::make_tuple(0x1 << 0, "FIN", PK_STRID("fin") ),
      };

      Property flags(PK_STRID("flags"), flag);
      std::string flagSummary;
      for (const auto& bit : flagTable) {
        bool on = std::get<0>(bit) & flag;
        Property flagBit(std::get<2>(bit), on);
        flagBit.setRange(reader.lastRange());
        flagBit.setError(reader.lastError());
        flags.addProperty(std::move(flagBit));
        if (on) {
          if (!flagSummary.empty()) flagSummary += ", ";
          flagSummary += std::get<1>(bit);
        }
      }
      flags.setSummary(flagSummary);
      flags.setRange(std::make_pair(12, 14));
      flags.setError(reader.lastError());

      Property window(PK_STRID("window"), reader.readBE<uint16_t>());
      window.setRange(reader.lastRange());
      window.setError(reader.lastError());

      Property checksum(PK_STRID("checksum"), reader.readBE<uint16_t>());
      checksum.setRange(reader.lastRange());
      checksum.setError(reader.lastError());

      Property urgent(PK_STRID("urgent"), reader.readBE<uint16_t>());
      urgent.setRange(reader.lastRange());
      urgent.setError(reader.lastError());

      Property options(PK_STRID("options"));
      options.setRange(reader.lastRange());
      options.setError(reader.lastError());

      size_t optionDataOffset = dataOffset * 4;
      size_t optionOffset = 20;
      while (optionDataOffset > optionOffset) {
        switch (layer->payload()[optionOffset]) {
          case 0:
            optionOffset = optionDataOffset;
            break;
          case 1:
            {
              Property opt(PK_STRID("nop"));
              opt.setRange(std::make_pair(optionOffset, optionOffset + 1));
              opt.setError(reader.lastError());
              options.addProperty(std::move(opt));
              optionOffset++;
            }
            break;
          case 2:
            {
              uint16_t size = fmt::readBE<uint16_t>(layer->payload(), optionOffset + 2);
              Property opt(PK_STRID("mss"), size);
              opt.setRange(std::make_pair(optionOffset, optionOffset + 4));
              opt.setError(reader.lastError());
              options.addProperty(std::move(opt));
              optionOffset += 4;
            }
            break;
          case 3:
            {
              uint8_t scale = fmt::readBE<uint8_t>(layer->payload(), optionOffset + 2);
              Property opt(PK_STRID("scale"), scale);
              opt.setRange(std::make_pair(optionOffset, optionOffset + 2));
              opt.setError(reader.lastError());
              options.addProperty(std::move(opt));
              optionOffset += 3;
            }
            break;

          case 4:
            {
              Property opt(PK_STRID("ackPerm"), true);
              opt.setRange(std::make_pair(optionOffset, optionOffset + 2));
              opt.setError(reader.lastError());
              options.addProperty(std::move(opt));
              optionOffset += 2;
            }
            break;

          // TODO: https://tools.ietf.org/html/rfc2018
          case 5:
            {
              uint8_t length = fmt::readBE<uint8_t>(layer->payload(), optionOffset + 1);
              Property opt(PK_STRID("selAck"), layer->payload().slice(optionOffset + 2, length));
              opt.setRange(std::make_pair(optionOffset, optionOffset + length));
              opt.setError(reader.lastError());
              options.addProperty(std::move(opt));
              optionOffset += length;
            }
            break;
          case 8:
            {
              uint32_t mt = fmt::readBE<uint32_t>(layer->payload(), optionOffset + 2);
              uint32_t et = fmt::readBE<uint32_t>(layer->payload(), optionOffset + 2 + sizeof(uint32_t));
              Property opt(PK_STRID("ts"), std::to_string(mt) + " - " + std::to_string(et));
              opt.setRange(std::make_pair(optionOffset, optionOffset + 10));
              opt.setError(reader.lastError());

              Property optmt(PK_STRID("mt"), mt);
              optmt.setRange(std::make_pair(optionOffset + 2, optionOffset + 6));
              optmt.setError(reader.lastError());

              Property optet(PK_STRID("et"), et);
              optet.setRange(std::make_pair(optionOffset + 6, optionOffset + 01));
              optet.setError(reader.lastError());

              opt.addProperty(std::move(optmt));
              opt.addProperty(std::move(optet));
              options.addProperty(std::move(opt));
              optionOffset += 10;
            }
            break;
          default:
            options.setError("Unknown option type");
            optionOffset = optionDataOffset;
            break;
        }
      }

      child.addProperty(std::move(src));
      child.addProperty(std::move(dst));
      child.addProperty(std::move(seq));
      child.addProperty(std::move(ack));
      child.addProperty(std::move(offset));
      child.addProperty(std::move(flags));
      child.addProperty(std::move(window));
      child.addProperty(std::move(checksum));
      child.addProperty(std::move(urgent));
      child.addProperty(std::move(options));

      const auto& payload = layer->payload().slice(optionDataOffset);
      const std::string &streamId = parentSrc->summary() +
        ":" + std::to_string(sourcePort) + "/" +
        parentDst->summary() + ":" + std::to_string(dstPort);
        
      child.setPayload(payload);
      child.setStreamId(streamId);

      // TODO:ALLOC
      return new Layer(std::move(child));
    }
  };

public:
  Dissector::WorkerPtr createWorker() override {
    return Dissector::WorkerPtr(new TCPDissector::Worker());
  }
  std::vector<strns> namespaces() const override {
    return std::vector<strns>{PK_STRNS("*tcp")};
  }
};

class TCPDissectorFactory final : public DissectorFactory {
public:
  DissectorPtr create(const SessionContext& ctx) const override {
    return DissectorPtr(new TCPDissector());
  }
};

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("factory").ToLocalChecked(),
    DissectorFactory::wrap(std::make_shared<TCPDissectorFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
