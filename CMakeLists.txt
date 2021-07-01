if(CONFIG_SDP)
zephyr_interface_library_named(SDP)

zephyr_include_directories(include)

zephyr_library()
zephyr_library_sources(
    src/datasample.c
    src/filter.c
    src/node.c
    src/proc_mgr.c
    src/sample_pool.c
    src/shell.c
    src/sink_mgr.c
    src/src_mgr.c
)

zephyr_library_link_libraries(SDP)
target_link_libraries(SDP INTERFACE zephyr_interface)
endif()
