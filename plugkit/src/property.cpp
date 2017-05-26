#include "property.hpp"
#include "wrapper/property.hpp"
#include <iostream>
#include <unordered_map>

namespace plugkit {

class Property::Private {
public:
  Private(const std::string &id, const std::string &name, const Variant &value);
  std::string id;
  std::string name;
  std::pair<uint32_t, uint32_t> range;
  std::string summary;
  std::string error;
  Variant value;
  std::vector<PropertyConstPtr> children;
  std::unordered_map<std::string, size_t> idMap;
};

Property::Private::Private(const std::string &id, const std::string &name,
                           const Variant &value)
    : id(id), name(name), value(value) {}

Property::Property() : d(new Private("", "", Variant())) {}

Property::Property(const std::string &id, const std::string &name,
                   const Variant &value)
    : d(new Private(id, name, value)) {}

Property::~Property() {}

Property::Property(Property &&prop) { this->d.reset(prop.d.release()); }

std::string Property::name() const { return d->name; }

void Property::setName(const std::string &name) { d->name = name; }

std::string Property::id() const { return d->id; }

void Property::setId(const std::string &id) { d->id = id; }

std::pair<uint32_t, uint32_t> Property::range() const { return d->range; }

void Property::setRange(const std::pair<uint32_t, uint32_t> &range) {
  d->range = range;
}

std::string Property::summary() const { return d->summary; }

void Property::setSummary(const std::string &summary) { d->summary = summary; }

std::string Property::error() const { return d->error; }

void Property::setError(const std::string &error) { d->error = error; }

Variant Property::value() const { return d->value; }

void Property::setValue(const Variant &value) { d->value = value; }

const std::vector<PropertyConstPtr> &Property::properties() const {
  return d->children;
}

PropertyConstPtr Property::propertyFromId(const std::string &id) const {
  auto it = d->idMap.find(id);
  if (it != d->idMap.end()) {
    return d->children[it->second];
  } else {
    return PropertyConstPtr();
  }
}

void Property::addProperty(const PropertyConstPtr &prop) {
  d->idMap[prop->id()] = d->children.size();
  d->children.push_back(prop);
}

void Property::addProperty(Property &&prop) {
  addProperty(std::make_shared<Property>(std::move(prop)));
}
}
