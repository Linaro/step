cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(step_foc)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/fusion_nodes.c)
target_sources(app PRIVATE src/imu_data_producer.c)

