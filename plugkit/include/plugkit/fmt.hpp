#ifndef PLUGKIT_FMT_H
#define PLUGKIT_FMT_H

#include "slice.hpp"
#include "types.hpp"
#include "export.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace plugkit {
namespace fmt {

template <class S> class Reader {
public:
  Reader(const S &slice);
  S slice(size_t length);
  S slice();
  template <class T> T readLE();
  template <class T> T readBE();
  template <class T> void skip();
  void skip(size_t length);
  Range lastRange() const;
  std::string lastError() const;

private:
  S slice_;
  size_t offset_;
  Range lastRange_;
  const char *lastError_;
};

template <class S>
Reader<S>::Reader(const S &slice)
    : slice_(slice), offset_(0), lastRange_(), lastError_(nullptr) {}

template <class S> S Reader<S>::slice(size_t length) {
  if (lastError_)
    return S();
  if (offset_ + length > slice_.length()) {
    lastRange_.begin = 0;
    lastRange_.end = 0;
    lastError_ = "unexpected EOS";
    return S();
  }
  const S &s = slice_.slice(offset_, length);
  lastRange_.begin = offset_;
  lastRange_.end = offset_ + length;
  offset_ += length;
  return s;
}

template <class S> S Reader<S>::slice() {
  return slice_.slice(offset_, slice_.length() - offset_);
}

template <class S> template <class T> T Reader<S>::readLE() {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  if (lastError_)
    return T();
  if (offset_ + sizeof(T) > slice_.length()) {
    lastRange_.begin = 0;
    lastRange_.end = 0;
    lastError_ = "unexpected EOS";
    return T();
  }
  const T &value = *reinterpret_cast<const T *>(slice_.data() + offset_);
  lastRange_.begin = offset_;
  lastRange_.end = offset_ + sizeof(T);
  offset_ += sizeof(T);
  return value;
}

template <class S> template <class T> T Reader<S>::readBE() {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  if (lastError_)
    return T();
  if (offset_ + sizeof(T) > slice_.length()) {
    lastRange_.begin = 0;
    lastRange_.end = 0;
    lastError_ = "unexpected EOS";
    return T();
  }
  char data[sizeof(T)];
  const char *begin = slice_.data() + offset_;
  std::reverse_copy(begin, begin + sizeof(T), data);
  const char *alias = data;
  const T &value = *reinterpret_cast<const T *>(alias);
  lastRange_.begin = offset_;
  lastRange_.end = offset_ + sizeof(T);
  offset_ += sizeof(T);
  return value;
}

template <class S> template <class T> void Reader<S>::skip() { readLE<T>(); }

template <class S> void Reader<S>::skip(size_t length) {
  if (lastError_)
    return;
  if (offset_ + length > slice_.length()) {
    lastRange_.begin = 0;
    lastRange_.end = 0;
    lastError_ = "unexpected EOS";
  }
  lastRange_.begin = offset_;
  lastRange_.end = offset_ + length;
  offset_ += length;
}

template <> template <> PLUGKIT_EXPORT uint8_t Reader<Slice>::readBE<uint8_t>();

template <> template <> PLUGKIT_EXPORT int8_t Reader<Slice>::readBE<int8_t>();

template <class S> Range Reader<S>::lastRange() const { return lastRange_; }

template <class S> std::string Reader<S>::lastError() const {
  return lastError_ ? lastError_ : std::string();
}

template <class T, class S> T readLE(const S &slice, size_t offset = 0) {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  return *reinterpret_cast<const T *>(slice.data() + offset);
}

template <class T, class S> T readBE(const S &slice, size_t offset = 0) {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  char data[sizeof(T)];
  const char *begin = slice.data() + offset;
  std::reverse_copy(begin, begin + sizeof(T), data);
  const char *alias = data;
  return *reinterpret_cast<const T *>(alias);
}

template <class S>
std::string toHex(const S &slice, int group = 0, int width = 2,
                  char sep = ':') {
  std::stringstream stream;
  stream << std::hex << std::setfill('0');
  for (size_t i = 0; i < slice.length(); ++i) {
    stream << std::setw(width) << +static_cast<uint8_t>(slice[i]);
    if (group > 0 && (i + 1) % group == 0 && i < slice.length() - 1) {
      stream << sep;
    }
  }
  return stream.str();
}

template <class S>
std::string toDec(const S &slice, int group = 0, int width = 0,
                  char sep = '.') {
  std::stringstream stream;
  stream << std::dec << std::setfill('0');
  for (size_t i = 0; i < slice.length(); ++i) {
    stream << std::setw(width) << +static_cast<uint8_t>(slice[i]);
    if (group > 0 && (i + 1) % group == 0 && i < slice.length() - 1) {
      stream << sep;
    }
  }
  return stream.str();
}

template <class S> std::string range(const S &base, const S &slice) {
  const char *baseBegin = base.data();
  const char *baseEnd = baseBegin + base.size();
  const char *sliceBegin = slice.data();
  const char *sliceEnd = sliceBegin + slice.length();
  if (baseBegin <= sliceBegin && sliceBegin < baseEnd && baseBegin < sliceEnd &&
      sliceEnd <= baseEnd) {
    std::string begin;
    std::string end;
    if (baseBegin != sliceBegin) {
      begin = std::to_string(sliceBegin - baseBegin);
    }
    if (baseEnd != sliceEnd) {
      end = std::to_string(sliceBegin - baseBegin + slice.length());
    }
    return begin + ":" + end;
  }
  return std::string();
}

template <class M>
typename M::mapped_type
enums(const M &table, const typename M::key_type &value,
      const typename M::mapped_type &defval = typename M::mapped_type()) {
  const auto &it = table.find(value);
  if (it != table.end()) {
    return it->second;
  }
  return defval;
}
}
}

#endif
