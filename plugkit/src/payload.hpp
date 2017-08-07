#ifndef PLUGKIT_PAYLOAD_HPP
#define PLUGKIT_PAYLOAD_HPP

#include "payload.h"
#include "types.hpp"
#include "slice.h"
#include "token.h"
#include <memory>
#include <vector>

namespace plugkit {

class Payload {
public:
  Payload(const Slice &view);
  ~Payload();
  Slice data() const;

  const std::vector<const Property *> &properties() const;
  const Property *propertyFromId(Token id) const;
  void addProperty(const Property *prop);

private:
  Payload(const Payload &payload) = delete;
  Payload &operator=(const Payload &payload) = delete;

private:
  Slice mData;
  std::vector<const Property *> mProperties;
};
}

#endif
