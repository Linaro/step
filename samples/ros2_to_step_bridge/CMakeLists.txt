cmake_minimum_required(VERSION 3.13.1)
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/micro_ros_zephyr_module/modules/libmicroros)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(step_ros)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/sensor_node.c)
target_sources(app PRIVATE src/imu_data_producer.c)
target_sources(app PRIVATE src/zsl_fusion_config.c)
