cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(step_sub)

target_sources(app PRIVATE src/driver.c)
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/nodes.c)
