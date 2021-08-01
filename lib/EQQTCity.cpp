#include "EQQTCity.h"

const char* toString(EQQTCity city) {
#define QQT_MAP_CITY(identifier, num, uiStr)                                   \
  case EQQTCity::identifier:                                                   \
    return #identifier;
  switch (city) {
  default:
    return "";
#include "EQQTCityRaw.h"
  }
#undef QQT_MAP_CITY
}
const char* toDescription(EQQTCity city) {
#define QQT_MAP_CITY(identifier, num, uiStr)                                   \
  case EQQTCity::identifier:                                                   \
    return uiStr;
  switch (city) {
  default:
    return "";
#include "EQQTCityRaw.h"
  }
#undef QQT_MAP_CITY
}
const std::initializer_list<EQQTCity>& getAllQQTCities() {
  static std::initializer_list<EQQTCity> il = {
#define QQT_MAP_CITY(id, num, uiStr) EQQTCity::id,
#include "EQQTCityRaw.h"
#undef QQT_MAP_CITY
  };
  return il;
}
