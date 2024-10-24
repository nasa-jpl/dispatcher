#ifndef SCRIPT_ITEM_H_
#define SCRIPT_ITEM_H_

#include "dispatcher/item.h"

namespace dispatcher
{

class ScriptItem : public Item
{
  Q_OBJECT;  // must be included to add qt meta information

 public:
  explicit ScriptItem(QWidget* parent, dispatcher::DispatcherNode* ros_node_,
                      const YAML::Node&);
  ~ScriptItem();

 public slots:
  void StartCb();

 private:
  std::string icon_;
  QLabel*     label_        = nullptr;
  bool        use_terminal_ = true;
};

}  // namespace dispatcher

#endif
