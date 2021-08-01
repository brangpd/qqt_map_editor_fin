#pragma once

#include <initializer_list>

enum class EQQTCity
{
#define QQT_MAP_CITY(identifier, num, uiStr) identifier = num,
#include "EQQTCityRaw.h"
#undef QQT_MAP_CITY
};

extern const char* toString(EQQTCity city);
extern const char* toDescription(EQQTCity city);
constexpr EQQTCity allQQTCities[] = {
#define QQT_MAP_CITY(identifier, num, uiStr) EQQTCity:: identifier,
#include "EQQTCityRaw.h"
#undef QQT_MAP_CITY
};
