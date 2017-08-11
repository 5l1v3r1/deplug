#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include "token.h"
#include "range.h"
#include "export.h"

PLUGKIT_NAMESPACE_BEGIN

struct Variant;
typedef struct Variant Variant;

struct Property;
typedef struct Property Property;

/// Gets id
PLUGKIT_EXPORT Token Property_id(const Property *prop);

/// Finds the first child property with the given id and returns it.
///
/// If no property is found, returns nullptr.
PLUGKIT_EXPORT const Property *Property_propertyFromId(const Property *prop,
                                                       Token id);

/// Gets range
PLUGKIT_EXPORT Range Property_range(const Property *prop);

/// Sets range
PLUGKIT_EXPORT void Property_setRange(Property *prop, Range range);

/// Gets type
PLUGKIT_EXPORT Token Property_type(const Property *prop);

/// Sets type
PLUGKIT_EXPORT void Property_setType(Property *prop, Token type);

/// Gets value as a const pointer
PLUGKIT_EXPORT const Variant *Property_value(const Property *prop);

/// Gets value as a mutable pointer
PLUGKIT_EXPORT Variant *Property_valueRef(Property *prop);

PLUGKIT_NAMESPACE_END

#endif
