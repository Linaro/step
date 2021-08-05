# Copyright (c) 2021 Linaro
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_STEP)
zephyr_interface_library_named(STEP)

zephyr_include_directories(include)

zephyr_library()
zephyr_library_sources(
    src/cache.c
    src/filter.c
    src/measurement.c
    src/node.c
    src/proc_mgr.c
    src/sample_pool.c
)

zephyr_library_link_libraries(STEP)
target_link_libraries(STEP INTERFACE zephyr_interface)
endif()
