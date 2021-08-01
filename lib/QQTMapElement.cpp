#include "QQTMapElement.h"

#include <algorithm>

using namespace std;

bool QQTMapElement::isGround() const {
  return ranges::all_of(attributes, [](auto attr) { return attr == 0xf0f; });
}
bool QQTMapElement::canBeHidden() const {
  return ranges::any_of(attributes, [](auto attr) { return attr & 0x1010; });
}
