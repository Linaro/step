# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_pipeline)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/sdp/src/filter_engine.c)
target_sources(app PRIVATE src/sdp/src/proc_mgr.c)
target_sources(app PRIVATE src/sdp/src/sample_pool.c)
target_sources(app PRIVATE src/sdp/src/sink_mgr.c)
target_sources(app PRIVATE src/sdp/src/src_mgr.c)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/sdp/include)
