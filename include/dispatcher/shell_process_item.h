#ifndef SHELL_PROCESS_ITEM_H_
#define SHELL_PROCESS_ITEM_H_

#include "dispatcher/process_item.h"

namespace dispatcher
{

/*! @brief Process item that infers liveness from a shell process name. */
class ShellProcessItem : public ProcessItem
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs a shell-backed process item. */
  explicit ShellProcessItem(QWidget* parent, DispatcherNode*, const YAML::Node&,
                            QGridLayout*);
  /*! @brief Destroys the shell process item. */
  ~ShellProcessItem();

  /*! @brief Polls shell process liveness and updates the item status indicator. */
  void Process();

 public slots:
  /*! @brief Starts the configured shell process. */
  void StartCb();
  /*! @brief Stops the configured shell process. */
  void StopCb();
  /*! @brief Opens a terminal attached to the shell process tmux session. */
  void TerminalCb();
};

}  // namespace dispatcher

#endif
