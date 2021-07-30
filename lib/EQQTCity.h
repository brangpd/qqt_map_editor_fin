#ifndef QQT_MAP_EDITOR_FIN_LIB_EQQTCity_H
#define QQT_MAP_EDITOR_FIN_LIB_EQQTCity_H

enum class EQQTCity {
#define QQT_MAP_CITY(identifier, num, uiStr) identifier = num,
#include "EQQTCityRaw.h"
#undef QQT_MAP_CITY
};

inline const char *toString(EQQTCity mapCategory) {
#define QQT_MAP_CITY(identifier, num, uiStr)                                   \
  case EQQTCity::identifier:                                                   \
    return #identifier;
  switch (mapCategory) {
  default:
    return "";
#include "EQQTCityRaw.h"
  }
#undef QQT_MAP_CITY
}

inline const char *toDescription(EQQTCity mapCategory) {
#define QQT_MAP_CITY(identifier, num, uiStr)                                   \
  case EQQTCity::identifier:                                                   \
    return uiStr;
  switch (mapCategory) {
  default:
    return "";
#include "EQQTCityRaw.h"
  }
#undef QQT_MAP_CITY
}

inline constexpr EQQTCity allQQTCities[] = {
#define QQT_MAP_CITY(id, num, uiStr) EQQTCity::id,
#include "EQQTCityRaw.h"
#undef QQT_MAP_CITY
};

#endif // QQT_MAP_EDITOR_FIN_LIB_EQQTCity_H
