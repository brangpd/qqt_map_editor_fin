#pragma once

enum class EQQTGameMode
{
#define QQT_GAME_MODE(identifier, num, uiStr) identifier = num,
#include "QQTGameModeRaw.h"
#undef QQT_GAME_MODE
};

extern const char* toString(EQQTGameMode mode);
extern const char* toDescription(EQQTGameMode mode);
constexpr EQQTGameMode allQQTGameModes[] = {
#define QQT_GAME_MODE(identifier, num, uiStr) EQQTGameMode:: identifier,
#include "QQTGameModeRaw.h"
#undef QQT_GAME_MODE
};
