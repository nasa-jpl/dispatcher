#include "dispatcher/config.h"
#include "dispatcher/dispatcher_widget.h"

#include <QApplication>
#include <iostream>

#include <rclcpp/rclcpp.hpp>

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);
  QApplication app(argc, argv);
  Q_INIT_RESOURCE(icons);
  dispatcher::DispatcherWidget w;
  w.show();
  return app.exec();
}
