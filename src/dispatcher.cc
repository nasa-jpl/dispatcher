#include "dispatcher/dispatcher_widget.h"
#include "dispatcher/config.h"

#include <QApplication>
#include <iostream>

#include <rclcpp/rclcpp.hpp>

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  QApplication app(argc, argv);
  std::cout << "dispatcher v" << dispatcher::version() << 
    " commit: " << dispatcher::commit() <<
    " branch: " << dispatcher::branch() << std::endl;
  dispatcher::DispatcherWidget w;
  w.show();
  return app.exec();
}
