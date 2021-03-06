#ifndef DISPATCH_ITEM_H_
#define DISPATCH_ITEM_H_

#include <QGroupBox>
#include <QLabel>
#include <QWidget>

#include <yaml-cpp/yaml.h>

namespace dispatcher {

class DispatcherWidget;

class DispatchItem : public QWidget {

  Q_OBJECT // must be included to add qt meta information

public:
  explicit DispatchItem(QWidget* parent, const YAML::Node&);
  ~DispatchItem();

private slots:

private:

  DispatcherWidget* dispatcher_ = nullptr;
  std::string name_;
  std::string node_name_;
  std::string node_namespace_;
  std::string cmd_;
  int index_ = -1;

signals:
};

}

#endif
