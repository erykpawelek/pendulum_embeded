#pragma once

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

#include "furuta_control/shared_interface/spi_interface.h"

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
  // Transition: Unconfigured -> Inactive (declare params, load ONNX model, create pubs/subs)
  CallbackReturn on_configure(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Inactive -> Active (enable publishers, start real-time thread)
  CallbackReturn on_activate(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Active -> Inactive (stop real-time thread, send stop frame)
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Inactive -> Unconfigured (release ONNX session, reset publishers/subscribers)
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State & previous_state) override;

  // Transition: Any state -> Finalized (stop motor and tear down)
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State & previous_state) override;

private:

};

}  // namespace furuta_control
