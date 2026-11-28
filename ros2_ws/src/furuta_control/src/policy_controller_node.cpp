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

  // TODO: Start the dedicated real-time thread (SCHED_FIFO, pinned core) that runs the
  //       50 Hz SPI exchange + inference. It must not run inside the executor.

  return CallbackReturn::SUCCESS;
}

CallbackReturn PolicyControllerNode::on_deactivate(const rclcpp_lifecycle::State & previous_state)
{
  RCLCPP_INFO(get_logger(), "Deactivating PolicyControllerNode...");

  // TODO: Stop the real-time thread, then send a stop frame (control_frame_make_stop())
  //       so the motor halts before the publisher is disabled

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

  // TODO: Stop the real-time thread and send a stop frame (control_frame_make_stop()).
  //       Even if this never runs (crash, kill -9) the STM32 watchdog stops the motor
  //       after SPI_WATCHDOG_TIMEOUT_MS.

  return CallbackReturn::SUCCESS;
}

}  // namespace furuta_control
