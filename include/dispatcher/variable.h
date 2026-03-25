#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <map>
#include <vector>

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QWidget>

#include <yaml-cpp/yaml.h>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <std_msgs/msg/empty.hpp>

namespace dispatcher
{
class DispatcherWidget;
class DispatcherNode;

/*! @brief Widget that exposes a named variable for command substitution. */
class Variable : public QWidget
{
  Q_OBJECT  // must be included to add qt meta information

 public:
  /*! @brief Constructs a variable selector from its YAML definition. */
  explicit Variable(QWidget* parent, dispatcher::DispatcherNode* ros_node_,
                    const YAML::Node&);
  /*! @brief Destroys the variable widget. */
  ~Variable();

  /*! @brief Enables or disables the variable selector UI. */
  void Enable(bool);
  /*! @brief Returns the variable name used in `$VARIABLE` substitutions. */
  std::string GetName();
  /*! @brief Returns the currently selected variable value. */
  std::string GetValue();

 public slots:

 private:
  DispatcherWidget* dispatcher_ = nullptr;
  DispatcherNode*   ros_node_   = nullptr;
  QComboBox*        combo_box_  = nullptr;

  std::string                         name_;
  std::vector<std::string>            choices_;
  int                                 index_          = -1;
  QLabel*                             label_          = nullptr;

 signals:
};

}  // namespace dispatcher

#endif
