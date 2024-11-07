#ifndef PROCESS_ITEM_H_
#define PROCESS_ITEM_H_

#include <QGridLayout>
#include <QPushButton>

#include "dispatcher/item.h"

namespace dispatcher
{

class ProcessItem : public Item
{
  Q_OBJECT;  // must be included to add qt meta information

 public:
  explicit ProcessItem(QWidget* parent, DispatcherNode*, const YAML::Node&,
                       QGridLayout*);
  ~ProcessItem();

  // get/set methods
  bool        is_online() { return online_; }
  std::string get_tmux_name() { return tmux_name_; };
  std::string get_name() { return name_; };
  bool        is_checked();

  void         SetEnabled(bool);
  void         SetVisible(bool);
  virtual void Process() = 0;
  void         UpdateConfiguration();
  bool         PrepareTmuxSession();
  bool         TmuxKillSession();
  bool         TmuxNewSession();
  void         TmuxSendKeys(std::string cmd_str);
  bool         TmuxHasSession();
  bool         TmuxHasLocalSession();

 public slots:
  void StartCb();
  void StopCb();
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
