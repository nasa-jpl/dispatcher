#ifndef DISPATCHER_WIDGET_H_
#define DISPATCHER_WIDGET_H_

#include <map>
#include <vector>

#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QWidget>
#include <QtGui>
#include <QTimer>

#include <casah_node/casah_node.hpp>

#include "dispatcher/dispatch_item.h"

namespace dispatcher {

  std::string version();
  std::string branch();
  std::string commit();  
  
class DispatcherWidget : public QWidget, CasahNode {

  Q_OBJECT // must be included to add qt meta information

public:
  explicit DispatcherWidget(QWidget* parent = 0);
  ~DispatcherWidget();
  virtual void Process() override;

private slots:

private:
  QTimer* timer_ = nullptr;
  std::string dispatcher_config_path_;
  std::vector<dispatcher::DispatchItem*> dispatch_items_;

  QSize minimumSizeHint() const {
    return QSize(50, 50);
  }
  QSize sizeHint() const {
    return QSize(700, 500);
  }
  void initializeLayout();
  void parseConfig(const std::string&);

signals:
};

}

#endif
