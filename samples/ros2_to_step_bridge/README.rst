.. step-fusion-sample:

Secure Telemetry Pipeline (STeP) ROS2 example
##############################################

Overview
********
This sample continuously sends fusion IMU data which is processed by a STeP processor
node to a ROS2 instance through the micro ROS communication bridge using USB-CDC as transport.
The data is send using standard ROS2 geometry_msgs/quaterion and could be
visualized into ROS2 RVIZ or Plotjuggler plugins.

Requirements
************
This sample assumes the user has some exposition to ROS2 and its command line tools.

This sample requires you to install ROS2 and micro-ROS additions to get
everything running, please refer the the RO2 getting starter: https://docs.ros.org/en/humble/index.html

Once user gets ROS2 in place into a laptop or a single computer board, just build this sample and connect
the USB of target board into the laptop or the single board computer where ROS2 runs.

Building and Flashing
*********************
Using this sample with the supported ST U585 IoT Discovery board does not 
require special instructions, just use west command as shown below:

.. code-block:: console

    west build  -p always -b b_u585i_iot02a  modules/lib/step/samples/ros2_to_step_bridge

Flashing also does not require special steps, just type:

.. code-block:: console

    west flash

Expected result
***************

After the firmware gets flashed, in a terminal of your ROS2 installation launch the
microXRCE-DDS agent:

.. code-block:: console
    micro-xrce-dds-agent serial --dev [YOUR_BOARD_PORT] -v6

On another terminal prepare ROS2 and run its topic utils to the device:
.. code-block:: console
    ros2 topic list

You should see the topic where the board publishes the data:

.. code-block:: console
    rosout
    sensor/step/imu/quaternion

As additional steps users are able to write ROS2 nodes to subscribe for data sent by the
board and do ROS2 local processing on it, or visualize data using for example Plotjuggler
for example:

.. code-block:: console
    ros2 run plotjuggler plotjuggler

The command above assume that Plotjuggler was installed after ROS2 gets in place.
