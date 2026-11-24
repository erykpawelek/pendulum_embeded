#pragma once

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

namespace furuta_control
{

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class PolicyControllerNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  // Explicit constructor accepting NodeOptions for parameter overrides and future composition
  explicit PolicyControllerNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~PolicyControllerNode() override = default;

protected:
  // Transition: Unconfigured -> Inactive (load ONNX model, declare params, create pubs/subs)
  CallbackReturn on_configure(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Inactive -> Active (enable publisher, start inference loop)
  CallbackReturn on_activate(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Active -> Inactive (send zero velocity command, stop inference)
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Inactive -> Unconfigured (release resources, reset smart pointers)
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Any state -> Finalized (safe emergency stop and teardown)
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State & previous_state) override;

private:

};

}  // namespace furuta_control