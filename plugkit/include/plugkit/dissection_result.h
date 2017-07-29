#ifndef PLUGKIT_DISSECTION_RESULT_H
#define PLUGKIT_DISSECTION_RESULT_H

#include "token.h"

extern "C" {

namespace plugkit {
class Layer;
}
using namespace plugkit;

struct DissectionResult {
  class Layer *child;
  char streamIdentifier[256];
  Token layerHints[8];
};
}

#endif
