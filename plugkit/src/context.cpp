#include "context.h"

void *Context_alloc(Context *ctx, size_t size) {
  // zero initialized
  return new char[size]();
}

void Context_dealloc(Context *ctx, void *ptr) {
  delete[] static_cast<char *>(ptr);
}
