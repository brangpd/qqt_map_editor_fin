#ifndef QQT_MAP_EDITOR_FIN_LIB_EQQTGAMEMODE_H
#define QQT_MAP_EDITOR_FIN_LIB_EQQTGAMEMODE_H

enum class EQQTGameMode {
#define QQT_GAME_MODE(identifier, num, uiStr) identifier = num,
#include "QQTGameModeRaw.h"
#undef QQT_GAME_MODE
};

inline const char *toString(EQQTGameMode mode) {
#define QQT_GAME_MODE(identifier, num, uiStr)                                  \
  case EQQTGameMode::identifier:                                               \
    return #identifier;
  switch (mode) {
  default:
    return "";
#include "QQTGameModeRaw.h"
  }
#undef QQT_GAME_MODE
}

inline const char *toDescription(EQQTGameMode mode) {
#define QQT_GAME_MODE(identifier, num, uiStr)                                  \
  case EQQTGameMode::identifier:                                               \
    return uiStr;
  switch (mode) {
  default:
    return "";
#include "QQTGameModeRaw.h"
  }
#undef QQT_GAME_MODE
}

inline constexpr EQQTGameMode allQQTGameModes[] = {
#define QQT_GAME_MODE(id, num, uiStr) EQQTGameMode::id,
#include "QQTGameModeRaw.h"
#undef QQT_GAME_MODE
};

#endif // QQT_MAP_EDITOR_FIN_LIB_EQQTGAMEMODE_H
