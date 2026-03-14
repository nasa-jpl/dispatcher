#ifndef SHELL_PROCESS_ITEM_H_
#define SHELL_PROCESS_ITEM_H_

#include "dispatcher/process_item.h"

namespace dispatcher
{

class ShellProcessItem : public ProcessItem
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  explicit ShellProcessItem(QWidget* parent, DispatcherNode*, const YAML::Node&,
                            QGridLayout*);
  ~ShellProcessItem();

  // Uses pgrep to poll for aliveness of shell process
  void Process();

 public slots:
  void StartCb();
  void StopCb();
  void TerminalCb();
};

}  // namespace dispatcher

#endif
