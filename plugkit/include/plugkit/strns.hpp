#ifndef PLUGKIT_STRNS_H
#define PLUGKIT_STRNS_H

#include "strid.hpp"
#include <array>
#include <functional>

namespace plugkit {

struct strns {
  enum flag { strns_sub_layer = 1 };

  std::array<strid, 3> id;

  constexpr strns() : id{{strid(), strid(), strid()}} {}
  constexpr strns(strid id1, strid id2, strid id3) : id{{id1, id2, id3}} {}
  constexpr strns(strid id1, strid id2) : id{{strid(), id1, id2}} {}
  constexpr strns(strid id1) : id{{strid(), strid(), id1}} {}
  std::string str() const;
  strid primary() const;
  bool empty() const;
  bool match(const strns &other) const;
  bool operator==(const strns &other) const;
};

inline std::string strns::str() const {
  std::string buf;
  for (size_t i = 0; i < id.size(); ++i) {
    if (!id[i].empty()) {
      if (id[i].tag())
        buf += "*";
      buf += id[i].str();
      if (i < id.size() - 1)
        buf += " ";
    }
  }
  return buf;
}

inline strid strns::primary() const {
  for (int i = id.size() - 1; i >= 0; --i) {
    if (!id[i].tag())
      return id[i];
  }
  return strid();
}

inline bool strns::empty() const {
  for (strid n : id) {
    if (!n.empty())
      return false;
  }
  return true;
}

inline bool strns::match(const strns &other) const {
  for (int i = id.size() - 1; i >= 0; --i) {
    if (!other.id[i].empty() && other.id[i].id != id[i].id)
      return false;
  }
  return true;
}

inline bool strns::operator==(const strns &other) const {
  for (size_t i = 0; i < id.size(); ++i) {
    if (id[i].id != other.id[i].id)
      return false;
  }
  return true;
}

template <size_t len> constexpr strid strid_ns_(const char *str) {
  return (str[0] == '*' ? strid_<len - 1, 1>(str) : strid_<len, 0>(str));
}

#define PK_STRNS(id1, id2, id3)                                                \
  strns {                                                                      \
    strid_ns_<sizeof(id1) - 2>(id1), strid_ns_<sizeof(id2) - 2>(id2),          \
        strid_ns_<sizeof(id3) - 2>(id3)                                        \
  }
}

namespace std {
template <> struct hash<plugkit::strns> {
  size_t operator()(const plugkit::strns &ns) const {
    size_t h = 0;
    for (plugkit::strid id : ns.id) {
      h ^= hash<uint64_t>()(id.id);
    }
    return h;
  }
};
}

#endif
