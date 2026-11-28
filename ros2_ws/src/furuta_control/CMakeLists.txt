cmake_minimum_required(VERSION 3.8)
project(furuta_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# 1. Dependencies
# ---------------------------------------------------------------------------
# ROS 2 packages: found through ament (they provide imported CMake targets).
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(std_msgs REQUIRED)

# ONNX Runtime: NOT a ROS package. It is installed system-wide on the Raspberry Pi
# (e.g. unpacked to /opt/onnxruntime or /usr/local). Point CMake to it with
#   colcon build --cmake-args -DONNXRUNTIME_ROOT=/opt/onnxruntime
# or leave it unset if it lives in a standard system prefix.
set(ONNXRUNTIME_ROOT "" CACHE PATH "Install prefix of ONNX Runtime (contains include/ and lib/)")

find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
  HINTS ${ONNXRUNTIME_ROOT}/include
  PATH_SUFFIXES onnxruntime)
find_library(ONNXRUNTIME_LIBRARY onnxruntime
  HINTS ${ONNXRUNTIME_ROOT}/lib)

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIBRARY)
  message(FATAL_ERROR
    "ONNX Runtime not found. Install it and pass -DONNXRUNTIME_ROOT=<prefix> "
    "(include dir: '${ONNXRUNTIME_INCLUDE_DIR}', library: '${ONNXRUNTIME_LIBRARY}').")
endif()

# Wrap the plain library into an imported target so it is linked like any other dependency.
add_library(onnxruntime::onnxruntime SHARED IMPORTED)
set_target_properties(onnxruntime::onnxruntime PROPERTIES
  IMPORTED_LOCATION "${ONNXRUNTIME_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}")

# ---------------------------------------------------------------------------
# 2. Library with the node class (reusable by the executable and by tests)
# ---------------------------------------------------------------------------
add_library(${PROJECT_NAME}_core src/policy_controller_node.cpp)

target_include_directories(${PROJECT_NAME}_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

# Modern imported targets carry include dirs and transitive link deps of each package.
# (std_msgs is declared above but not linked until a message type is actually used.)
target_link_libraries(${PROJECT_NAME}_core PUBLIC
  rclcpp::rclcpp
  rclcpp_lifecycle::rclcpp_lifecycle
  onnxruntime::onnxruntime)

# ---------------------------------------------------------------------------
# 3. Executable: only main(), everything else lives in the library
# ---------------------------------------------------------------------------
add_executable(policy_controller src/main.cpp)
target_link_libraries(policy_controller PRIVATE ${PROJECT_NAME}_core)

# ---------------------------------------------------------------------------
# 4. Install: executable goes to lib/<pkg>/ so `ros2 run furuta_control policy_controller` works
# ---------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME}_core
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(TARGETS policy_controller
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
