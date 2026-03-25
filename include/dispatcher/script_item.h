#ifndef SCRIPT_ITEM_H_
#define SCRIPT_ITEM_H_

#include "dispatcher/item.h"

namespace dispatcher
{

/*! @brief Button-like dispatcher item that runs an arbitrary script command. */
class ScriptItem : public Item
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs a script item from its YAML button definition. */
  explicit ScriptItem(QWidget* parent, dispatcher::DispatcherNode* ros_node_,
                      const YAML::Node&);
  /*! @brief Destroys the script item. */
  ~ScriptItem();

 public slots:
  /*! @brief Executes the configured script command. */
  void StartCb();

 private:
  std::string icon_;
  QLabel*     label_        = nullptr;
  bool        use_terminal_ = true;
};

}  // namespace dispatcher

#endif
