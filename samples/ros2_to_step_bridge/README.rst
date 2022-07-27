.. step-fusion-sample:

Secure Telemetry Pipeline (STeP) ROS2 exammple
##############################################

Overview
********

Requirements
************


Building and Flashing
*********************
Using this sample with the supported ST U585 IoT Discovery board does not 
require special instructions, just use west command as shown below:

.. code-block:: console

    west build  -p always -b b_u585i_iot02a  modules/lib/step/samples/ros2_to_step_bridge

This will fail at the first time, because the rcutils, needs to find the Zephyr's own POSIX
clock implementation, after the first time fail, apply the patch on the rcutils using commands
below:

.. code-block:: console

    $ chmod u+x patch_rcutils.sh 

.. code-block:: console

    $ ./patch_rcutils.sh 

After this step rcutils may be able to find the correct POSIX clock implementation for Zephyr
, so just invoke build again:

.. code-block:: console

    west build

Flashing also does not require special steps, just type:

.. code-block:: console

    west flash

Expected result
***************

.. code-block:: console


.. code-block:: console


.. code-block:: console
