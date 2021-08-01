#ifndef QQT_MAP_EDITOR_FIN__MAPEDITCOMMAND_H
#define QQT_MAP_EDITOR_FIN__MAPEDITCOMMAND_H

class MapEditCommand {
public:
  virtual ~MapEditCommand() = default;
  /// 执行，也用于重做
  virtual void exec() = 0;
  /// 撤销
  virtual void undo() = 0;
};

#endif //QQT_MAP_EDITOR_FIN__MAPEDITCOMMAND_H
