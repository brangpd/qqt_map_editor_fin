#include "QQFDIMGReaderVer1.h"
#include "QQFDIMG.h"

#include "DefineIOMacros.h"

using namespace std;
bool QQFDIMGReaderVer1::read(std::istream &is, QQFDIMG &qqfdimg) {
  fwd(sizeof(int16_t) + sizeof(int32_t));

  readt(qqfdimg.nFrames, int32_t);
  readt(qqfdimg.nDirections, int32_t);
  readt(qqfdimg.xOffset, int32_t);
  readt(qqfdimg.yOffset, int32_t);
  readt(qqfdimg.wOrigin, int32_t);
  readt(qqfdimg.hOrigin, int32_t);

  auto &frames = qqfdimg.frames;
  frames.reserve(qqfdimg.nFrames);
  for (int i = 0, ie = qqfdimg.nDirections; i < ie; ++i) {
    for (int j = 0, je = qqfdimg.nFrames / qqfdimg.nDirections; j < je; ++j) {
      auto &frame = frames.emplace_back();
      fwd(sizeof(int32_t));
      readt(frame.xOffset, int32_t);
      readt(frame.yOffset, int32_t);
      fwd(sizeof(int32_t));
      readt(frame.w, int32_t);
      readt(frame.h, int32_t);
      fwd(sizeof(int32_t));
      int64_t cnt = frame.w * frame.h * 4;
      frame.img.resize(cnt);
      is.read(frame.img.data(), cnt);
    }
  }

  return true;
}
