#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include "variant.hpp"
#include "export.hpp"
#include <memory>
#include <vector>

namespace plugkit {

class Property;
using PropertyConstPtr = std::shared_ptr<const Property>;

class PLUGKIT_EXPORT Property final {
public:
  Property();
  Property(const char *id, const std::string &name,
           const Variant &value = Variant());
  Property(Property &&prop);
  ~Property();

  std::string name() const;
  void setName(const std::string &name);
  const char *id() const;
  void setId(const char *id);
  std::pair<uint32_t, uint32_t> range() const;
  void setRange(const std::pair<uint32_t, uint32_t> &range);
  std::string summary() const;
  void setSummary(const std::string &summary);
  std::string error() const;
  void setError(const std::string &error);
  Variant value() const;
  void setValue(const Variant &value);

  const std::vector<PropertyConstPtr> &properties() const;
  PropertyConstPtr propertyFromId(const char *id) const;
  void addProperty(const PropertyConstPtr &prop);
  void addProperty(Property &&prop);

private:
  Property(const Property &prop) = delete;
  Property &operator=(const Property &prop) = delete;

private:
  class Private;
  std::unique_ptr<Private> d;
};
}

#endif
