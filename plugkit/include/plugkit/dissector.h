#ifndef PLUGKIT_DISSECTOR_H
#define PLUGKIT_DISSECTOR_H

#include "export.h"
#include "token.h"
#include <stdbool.h>

PLUGKIT_NAMESPACE_BEGIN

struct Layer;
typedef struct Layer Layer;

struct Context;
typedef struct Context Context;

typedef struct Dissector Dissector;

typedef enum DissectorType {
  DISSECTOR_PACKET = 0,
  DISSECTOR_STREAM = 1
} DissectorType;

typedef void(AnalyzerFunc)(Context *ctx, void *worker, Layer *layer);
typedef void *(WokerFactoryAllocFunc)(Context *ctx);
typedef void(WokerFactoryDeallocFunc)(Context *ctx, void *);
typedef bool(ExpiryFunc)(Context *ctx, void *worker, uint32_t elapsed);

PLUGKIT_EXPORT Dissector *Dissector_create(DissectorType type);
PLUGKIT_EXPORT void Dissector_setAnalyzer(Dissector *diss, AnalyzerFunc *func);
PLUGKIT_EXPORT void
Dissector_setWorkerFactory(Dissector *diss, WokerFactoryAllocFunc *alloc,
                           WokerFactoryDeallocFunc *dealloc);
PLUGKIT_EXPORT void Dissector_setExpiry(Dissector *diss, ExpiryFunc *func);
PLUGKIT_EXPORT void Dissector_addLayerHint(Dissector *diss, Token hint);

PLUGKIT_NAMESPACE_END

#endif
