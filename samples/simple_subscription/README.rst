.. step-shell-sample:

Secure Telemetry Pipeline (STeP) Subscription Example
#####################################################

Overview
********

This sample demonstrates how to use the subscription API.
A STeP node chain is created, and the external application
subscribes to the node chain's completion event, republishing
the data to the node chain.

Requirements
************

Depending on the device you are testing with, you may need to enable or
disable the ``CONFIG_STDOUT_CONSOLE`` flag.

It should be set to ``y`` when using the emulator, and to ``n`` when running on
real hardware.

Building and Running
********************

To run this example on the **mps2_an521 (Cortex M33) emulator**, run the
following commands which will compile the application, run it on the emulator,
and output the result to the console:

.. code-block:: console

    $ cd samples/simple_subscription
    $ mkdir b && cd b
    $ cmake -GNinja -DBOARD=mps2_an521 ..
    $ ninja
    $ ninja run

To do the same thing using ``west``, run:

.. code-block:: console

   $ west build -p -b mps2_an521 samples/simple_subscription/ -t run

Press ``CTRL+A`` to exit QEMU.

To run the application on real hardware, typically outputting the results to the
serial port, you can try a variant of the following, adjusting ``-DBOARD``
as appropriate.

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS build zephyr-v3.0.0-3946-ga70ff60c4b91  ***
   Receving subscription callback from node 0 
   Die temp: 320.000000 [C] at 0 [ms]
   Receving subscription callback from node 0 
   Die temp: 320.000000 [C] at 0 [ms]
   Receving subscription callback from node 0 
   Die temp: 320.000000 [C] at 0 [ms]
   Receving subscription callback from node 0 
   Die temp: 320.000000 [C] at 0 [ms]
