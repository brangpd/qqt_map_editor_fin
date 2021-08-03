#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <QImage>

struct QQFDIMG
{
  struct Frame
  {
    QImage image;
    int32_t xOffset;
    int32_t yOffset;
    int32_t w;
    int32_t h;
  };

  int16_t version;
  int32_t xOffset;
  int32_t yOffset;
  int32_t wOrigin;
  int32_t hOrigin;
  uint32_t nFrames;
  uint32_t nDirections;
  std::vector<Frame> frames;
  QImage previewImage;

  bool read(std::istream &is);
  bool write(std::ostream &os) const;
};
