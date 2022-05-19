cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr)
project(step)

zephyr_include_directories($ENV{ZEPHYR_BASE}/kernel/include/)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
