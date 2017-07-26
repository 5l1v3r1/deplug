#include "stream_resolver.hpp"
#include <unordered_map>

namespace plugkit {

class StreamResolver::Private {
public:
  std::unordered_map<std::string, uint32_t> map;
  uint32_t counter = 0;
};

StreamResolver::StreamResolver() : d(new Private()) {}

StreamResolver::~StreamResolver() {}

void StreamResolver::resolve(std::pair<Layer *, std::string>* begin, size_t size)
{
  for (size_t i = 0; i < size; ++i) {
    const auto& pair = begin[i];
    const auto it = d->map.find(pair.second);
    if (it != d->map.end()) {

    } else {
      d->map[pair.second] = ++counter;
    }
  }
}

}
