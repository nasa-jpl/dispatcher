#ifndef PROCESS_ITEM_H_
#define PROCESS_ITEM_H_

#include <QGridLayout>
#include <QPushButton>

#include "dispatcher/item.h"

namespace dispatcher
{

/*! @brief Base class for dispatcher items that launch and monitor processes. */
class ProcessItem : public Item
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs a process-backed item from YAML and inserts its controls. */
  explicit ProcessItem(QWidget* parent, DispatcherNode*, const YAML::Node&,
                       QGridLayout*);
  /*! @brief Destroys the process item. */
  ~ProcessItem();

  /*! @brief Returns whether the item is currently considered online. */
  bool        is_online() { return online_; }
  /*! @brief Returns the tmux session name associated with the item. */
  std::string get_tmux_name() { return tmux_name_; };
  /*! @brief Returns the display name of the item. */
  std::string get_name() { return name_; };
  /*! @brief Returns whether the UI checkbox is checked or partially checked. */
  bool        is_checked();

  /*! @brief Enables, disables, or hides the process controls. */
  void         SetEnabled(bool);
  /*! @brief Performs item-specific monitoring work. */
  virtual void Process() = 0;
  /*! @brief Updates the active configuration and start-button tooltip. */
  void         UpdateConfiguration();
  /*! @brief Verifies tmux/session preconditions before starting the process. */
  bool         PrepareTmuxSession();
  /*! @brief Kills the tmux session associated with this item. */
  bool         TmuxKillSession();
  /*! @brief Creates a fresh tmux session for this item. */
  bool         TmuxNewSession();
  /*! @brief Sends a command to the tmux session for this item. */
  void         TmuxSendKeys(std::string cmd_str);
  /*! @brief Returns whether the configured tmux session currently exists. */
  bool         TmuxHasSession();
  /*! @brief Returns whether a local tmux session exists for this item. */
  bool         TmuxHasLocalSession();

 public slots:
  /*! @brief Starts the configured process command in tmux. */
  void StartCb();
  /*! @brief Sends the configured stop sequence to the running process. */
  void StopCb();
  /*! @brief Opens a terminal attached to the tmux session for this item. */
  void TerminalCb();

 protected:
  int          nticks_           = 0;
  bool         attach_on_start_  = false;
  bool         tmux_initialized_ = false;
  bool         enabled_          = true;
  std::string  tmux_name_;
  std::string  stop_tmux_cmd_;
  int          index_                     = -1;
  bool         online_                    = false;
  size_t       num_online_nodes_prev_     = 0;
  bool         use_environment_variables_ = true;
  QLabel*      label_                     = nullptr;
  QPixmap      red_status_icon_;
  QPixmap      orange_status_icon_;
  QPixmap      green_status_icon_;
  QPixmap      grey_status_icon_;
  QCheckBox*   check_box_ = nullptr;
  QPushButton* start_     = nullptr;
  QPushButton* stop_      = nullptr;
  QPushButton* terminal_  = nullptr;
};

}  // namespace dispatcher

#endif
