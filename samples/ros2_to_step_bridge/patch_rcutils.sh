#!/bin/sh

pushd micro_ros_zephyr_module/modules/libmicroros/micro_ros_src/src/rcutils
git apply ../../../../../../rcutils_time_patch.rcutils_time_patch
popd