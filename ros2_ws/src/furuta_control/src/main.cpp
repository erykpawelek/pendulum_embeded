#include <memory>

#include "furuta_control/policy_controller_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  // SingleThreadedExecutor runs one callback at a time, so lifecycle transitions and
  // other ROS callbacks never execute concurrently (no locking needed between them).
  // The hard real-time loop (SPI exchange + inference) will NOT run here: it gets its own
  // dedicated thread started in on_activate(), so the executor can never delay it.
  rclcpp::executors::SingleThreadedExecutor executor;
  auto node = std::make_shared<furuta_control::PolicyControllerNode>();

  // LifecycleNode requires passing its base interface to the executor
  executor.add_node(node->get_node_base_interface());
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
