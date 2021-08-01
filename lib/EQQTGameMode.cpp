#include "EQQTGameMode.h"

const char* toString(EQQTGameMode mode) {
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
const char* toDescription(EQQTGameMode mode) {
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
const std::initializer_list<EQQTGameMode>& getAllQQTGameModes() {
  static std::initializer_list<EQQTGameMode> il = {
#define QQT_GAME_MODE(id, num, uiStr) EQQTGameMode::id,
#include "QQTGameModeRaw.h"
#undef QQT_GAME_MODE
  };
  return il;
}
