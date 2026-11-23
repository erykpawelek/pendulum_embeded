#include "furuta_control/policy_controller_node.hpp"

#include <memory>

namespace furuta_control
{

PolicyControllerNode::PolicyControllerNode(const rclcpp::NodeOptions & options)
: rclcpp_lifecycle::LifecycleNode("policy_controller_node", options)
{
  // Keep the constructor lightweight; defer resource allocation to on_configure()
  RCLCPP_INFO(get_logger(), "PolicyControllerNode constructed (Unconfigured).");
}

CallbackReturn PolicyControllerNode::on_configure(const rclcpp_lifecycle::State & previous_state)
{
  (void)previous_state;
  RCLCPP_INFO(get_logger(), "Configuring PolicyControllerNode...");

  // TODO: Read parameters, initialize ONNX Runtime session, create publishers and subscribers

  return CallbackReturn::SUCCESS;
}

CallbackReturn PolicyControllerNode::on_activate(const rclcpp_lifecycle::State & previous_state)
{
  RCLCPP_INFO(get_logger(), "Activating PolicyControllerNode...");

  // Base class call automatically activates all managed LifecyclePublishers
  LifecycleNode::on_activate(previous_state);

  // TODO: Enable control flag or start control timer

  return CallbackReturn::SUCCESS;
}

CallbackReturn PolicyControllerNode::on_deactivate(const rclcpp_lifecycle::State & previous_state)
{
  RCLCPP_INFO(get_logger(), "Deactivating PolicyControllerNode...");

  // TODO: Publish a final zero-velocity command before disabling the publisher

  // Base class call automatically deactivates all managed LifecyclePublishers
  LifecycleNode::on_deactivate(previous_state);

  return CallbackReturn::SUCCESS;
}

CallbackReturn PolicyControllerNode::on_cleanup(const rclcpp_lifecycle::State & previous_state)
{
  (void)previous_state;
  RCLCPP_INFO(get_logger(), "Cleaning up PolicyControllerNode...");

  // TODO: Reset publishers, subscribers, and release ONNX session memory

  return CallbackReturn::SUCCESS;
}

CallbackReturn PolicyControllerNode::on_shutdown(const rclcpp_lifecycle::State & previous_state)
{
  (void)previous_state;
  RCLCPP_INFO(get_logger(), "Shutting down PolicyControllerNode...");

  // TODO: Ensure motor command is zeroed and release remaining resources

  return CallbackReturn::SUCCESS;
}

}  // namespace furuta_control

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  // Use SingleThreadedExecutor to guarantee deterministic, lock-free callback execution
  rclcpp::executors::SingleThreadedExecutor executor;
  auto node = std::make_shared<furuta_control::PolicyControllerNode>();

  // LifecycleNode requires passing its base interface to the executor
  executor.add_node(node->get_node_base_interface());
  executor.spin();

  rclcpp::shutdown();
  return 0;
}