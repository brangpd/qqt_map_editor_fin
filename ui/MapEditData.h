#ifndef MAPEDITDATA_H
#define MAPEDITDATA_H

#include <vector>

struct MapEditData {
  bool shouldShowTop;
  bool shouldShowBottom;
  bool shouldShowSpawnPoints;
  bool shouldShowGridLines;
  std::vector<int> mapElements;
};

#endif //MAPEDITDATA_H
