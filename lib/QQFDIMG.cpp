#include "QQFDIMG.h"

#include <iostream>
#include <QColor>

using namespace std;
#include "DefineIOMacros.h"
bool QQFDIMG::read(std::istream &is) {
  if (!is) {
    qWarning("输入流为空");
    return false;
  }
  // 判断文件头是否为 QQF DIMG
  char head[9];
  is.read(head, 8);
  head[8] = 0;
  if (head != "QQF\x1a"
    "DIMG"sv) {
    qWarning("QQFDIMG文件头格式错误");
    return false;
  }
  READT(version, int16_t);
  FWD(sizeof(int16_t) + sizeof(int32_t));

  READT(nFrames, uint32_t);
  READT(nDirections, uint32_t);
  READT(xOffset, int32_t);
  READT(yOffset, int32_t);
  READT(wOrigin, int32_t);
  READT(hOrigin, int32_t);

  if (nFrames <= 0 || nDirections <= 0) {
    qWarning("QQFDIMG帧数或方向数非正");
    return false;
  }
  nFrames = nDirections = 1;

  frames.clear();
  frames.reserve(nFrames);
  switch (version) {
  case 0:
    {
      for (uint32_t i = 0, ie = nDirections; i < ie; ++i) {
        for (uint32_t j = 0, je = nFrames / nDirections; j < je; ++j) {
          auto &frame = frames.emplace_back();
          FWD(sizeof(int32_t));
          READT(frame.xOffset, int32_t);
          READT(frame.yOffset, int32_t);
          FWD(sizeof(int32_t));
          READT(frame.w, int32_t);
          READT(frame.h, int32_t);
          FWD(sizeof(int32_t));
          auto cnt = static_cast<int64_t>(frame.w) * frame.h * 3;
          vector<unsigned char> buf(cnt);
          is.read(reinterpret_cast<char*>(buf.data()), cnt);
          size_t k = 0;
          QColor color;
          frame.image = QImage(frame.w, frame.h, QImage::Format_ARGB32);
          for (int row = 0, re = frame.h; row < re; ++row) {
            for (int col = 0, ce = frame.w; col < ce; ++col) {
              uint16_t rgb565 = buf[k * 2 + 1] << 8U | buf[k * 2];
              double r = (rgb565 & 0b1111100000000000U) >> 11U;
              r = r / (1U << 5U);
              double g = (rgb565 & 0b11111100000U) >> 5U;
              g = g / (1U << 6U);
              double b = rgb565 & 0b11111U;
              b = b / (1U << 5U);
              double a = (buf[k + 2 * frame.h * frame.w]) / 32.0;
              color.setRgbF(r, g, b, a);
              frame.image.setPixelColor(col, row, color);
              ++k;
            }
          }
        }
      }
    }
    break;
  case 1:
    {
      for (uint32_t i = 0, ie = nDirections; i < ie; ++i) {
        for (uint32_t j = 0, je = nFrames / nDirections; j < je; ++j) {
          auto &frame = frames.emplace_back();
          FWD(sizeof(int32_t));
          READT(frame.xOffset, int32_t);
          READT(frame.yOffset, int32_t);
          FWD(sizeof(int32_t));
          READT(frame.w, int32_t);
          READT(frame.h, int32_t);
          FWD(sizeof(int32_t));
          int64_t cnt = static_cast<int64_t>(frame.w) * frame.h * 4;
          // argb
          vector<unsigned char> buf(cnt);
          is.read(reinterpret_cast<char*>(buf.data()), cnt);
          size_t k = 0;
          QColor color;
          for (int row = 0, rowe = frame.h; row < rowe; ++row) {
            for (int col = 0, cole = frame.w; col < cole; ++col) {
              color.setRgb(buf[k + 2], buf[k + 1], buf[k], buf[k + 3]);
              frame.image.setPixelColor(col, row, color);
              k += 4;
            }
          }
        }
      }
    }
    break;
  default:
    qCritical("QQFDIMG未支持的版本%i", version);
    return false;
  }
  previewImage = frames.front().image;

  return true;
}
bool QQFDIMG::write(std::ostream &/*os*/) const {
  qCritical("写QQFDIMG没有实现");
  return false;
}
