#include "QQTMapElement.h"
#include <algorithm>
using namespace std;
EQQTCity QQTMapElement::getCity() const {
  return static_cast<EQQTCity>(id / 1000);
}
bool QQTMapElement::isGround() const {
  return ranges::all_of(attributes, [](auto &&attr) { return attr == 0xf0f; });
}
