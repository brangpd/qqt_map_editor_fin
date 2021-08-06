#pragma once

#include <vector>

class MapEditCommand
{
public:
  virtual ~MapEditCommand() = default;
  /// 执行，也用于重做
  virtual void exec() = 0;
  /// 撤销
  virtual void undo() = 0;
};
