#ifndef PLUGKIT_ERROR_H
#define PLUGKIT_ERROR_H

#include "token.h"

extern "C" {

namespace plugkit {

struct Error {
  Token type;
};
}
}

#endif
