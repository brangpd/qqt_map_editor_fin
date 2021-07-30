#ifndef QQT_MAP_EDITOR_FIN_LIB_QQFDIMG_H
#define QQT_MAP_EDITOR_FIN_LIB_QQFDIMG_H

#include <cstdint>
#include <string>
#include <vector>

struct QQFDIMG {
  struct Frame {
    std::string img;
    int32_t xOffset;
    int32_t yOffset;
    int32_t w;
    int32_t h;
  };
  int32_t xOffset;
  int32_t yOffset;
  int32_t wOrigin;
  int32_t hOrigin;
  int32_t nFrames;
  int32_t nDirections;
  std::vector<Frame> frames;
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQFDIMG_H
