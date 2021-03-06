#ifndef DISPATCHER_WIDGET_H_
#define DISPATCHER_WIDGET_H_

#include <map>
#include <vector>

#include <QGroupBox>
#include <QGridLayout>
#include <QWidget>
#include <QTimer>
#include <QSocketNotifier>

#include <casah_node/casah_node.hpp>
#include <rclcpp/node_interfaces/node_graph.hpp>

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
  QGridLayout* getLayout() {
    return layout_;
  }
  // Unix signal handlers.
  static void hupSignalHandler(int unused);
  static void termSignalHandler(int unused);

  const std::vector<std::pair<std::string, std::string>>& getOnlineNodes() {
    return online_nodes_;
  }

public slots:
  // Qt signal handlers.
  void handleSigHup();
  void handleSigTerm();
  virtual void Process() override;

private:
  static int sighupFd[2];
  static int sigtermFd[2];

  QSocketNotifier* snHup = nullptr;
  QSocketNotifier* snTerm = nullptr;

  QTimer* timer_ = nullptr;
  QGridLayout* layout_ = nullptr;
  std::string dispatcher_config_path_;
  std::vector<dispatcher::DispatchItem*> dispatch_items_;

  std::shared_ptr<rclcpp::node_interfaces::NodeGraph> node_graph_; 
  std::vector<std::pair<std::string, std::string>> online_nodes_;

  QSize minimumSizeHint() const {
    return QSize(30, 30);
  }
  QSize sizeHint() const {
    return QSize(30, 30);
  }
  void initializeLayout();
  void parseConfig(const std::string&);

signals:
};

}

#endif
