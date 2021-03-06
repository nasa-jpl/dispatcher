#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/dispatch_item.h"
#include "dispatcher/config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpacerItem>
#include <QSocketNotifier>
#include <QCoreApplication>

#include <rclcpp/node_interfaces/node_base_interface.hpp>

#include <yaml-cpp/yaml.h>

#include <cfw/cfw.h>

std::string dispatcher::version() {
  std::string version_major = std::to_string(DISPATCHER_VERSION_MAJOR);
  std::string version_minor = std::to_string(DISPATCHER_VERSION_MINOR);
  std::string version_patch = std::to_string(DISPATCHER_VERSION_PATCH);
  return version_major + std::string(".") + version_minor + std::string(".") +
         version_patch;
}

std::string dispatcher::branch() {
  return std::string(DISPATCHER_GIT_BRANCH);
}

std::string dispatcher::commit() {
  return std::string(DISPATCHER_GIT_COMMIT);
}

#if 0
static int setup_unix_signal_handlers() {
  struct sigaction hup, term;

  hup.sa_handler = dispatcher::DispatcherWidget::hupSignalHandler;
  sigemptyset(&hup.sa_mask);
  hup.sa_flags = 0;
  hup.sa_flags |= SA_RESTART;

  if (sigaction(SIGHUP, &hup, 0)) {
    return 1;
  }

  term.sa_handler = dispatcher::DispatcherWidget::termSignalHandler;
  sigemptyset(&term.sa_mask);
  term.sa_flags = 0;
  term.sa_flags |= SA_RESTART;

  if (sigaction(SIGTERM, &term, 0)) {
    return 2;
  }

  return 0;
}
#endif

/*!
@brief class constructor for DispatcherWidget application
*/
dispatcher::DispatcherWidget::DispatcherWidget(QWidget* parent)
  : QWidget(parent), CasahNode("dispatcher", "dispatcher", 2.0, "/logs", "INFO") {

#if 0
  setup_unix_signal_handlers();

  // set up signal handling
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd)) {
    qFatal("Couldn't create HUP socketpair");
  }

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
     qFatal("Couldn't create TERM socketpair");
  }

  snHup = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
  connect(snHup, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigHup()));
  snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
  connect(snTerm, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigTerm()));
#endif

  // declare ros parameters
  this->declare_parameter<std::string>("dispatcher_config_path", "");
  this->get_parameter("dispatcher_config_path", dispatcher_config_path_);

  timer_ = new QTimer;

  QGroupBox* main_box = new QGroupBox;
  main_box->setContentsMargins(QMargins(0, 0, 0, 0));

  layout_ = new QGridLayout(main_box);
  setLayout(layout_);

  CFW_INFO("Parsing config file: %s", dispatcher_config_path_.c_str());
  parseConfig(dispatcher_config_path_);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout_->addItem(spacer, layout_->rowCount(), 0);

   // setup signal and slot
  connect(timer_, SIGNAL(timeout()), this, SLOT(Process())); 
  double loop_period_ms = 1.0 / target_loop_rate_hz_ * 1000.0;
  CFW_INFO("Using loop period %f ms", loop_period_ms);
  timer_->start(loop_period_ms);

  node_graph_ = 
    std::make_shared<rclcpp::node_interfaces::NodeGraph>(
      this->get_node_base_interface().get());
}

void dispatcher::DispatcherWidget::parseConfig(const std::string& config) {

  YAML::Node root = YAML::LoadFile(config.c_str());
  const YAML::Node& nodes = root["nodes"];
  for (const auto& node : nodes) {
    dispatch_items_.push_back(new dispatcher::DispatchItem(this, node));
  }
}

/*!
@brief class destructor
*/
dispatcher::DispatcherWidget::~DispatcherWidget() {}


void dispatcher::DispatcherWidget::handleSigTerm()
{
#if 0
    snTerm->setEnabled(false);
    char tmp;
    read(sigtermFd[1], &tmp, sizeof(tmp));

    // do Qt stuff

    snTerm->setEnabled(true);
#endif
}


void dispatcher::DispatcherWidget::handleSigHup()
{
#if 0
    snHup->setEnabled(false);
    char tmp;
    read(sighupFd[1], &tmp, sizeof(tmp));

    // do Qt stuff

    snHup->setEnabled(true);
#endif
}

void dispatcher::DispatcherWidget::Process() {
  if(rclcpp::ok()) {
     online_nodes_ = node_graph_->get_node_names_and_namespaces();
     for(auto& node : online_nodes_) {
       std::cout << node.first << node.second << std::endl;
     }
     std::cout << std::endl;
  } else {
    CFW_INFO("SHUTTING DOWN");
    QCoreApplication::quit();
  }
}
