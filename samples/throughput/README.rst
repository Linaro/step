.. step-throughput-sample:

Secure Telemetry Pipeline (STeP) Throughput Example
###################################################

Overview
********

This sample provides basic throughput testing to determine how many measurements
per second can be processed with a minimal registry setup.

There are two common scenarios where throughput should be tested:

1. Using the measurement polling thread.
2. Manually processing measurements (direct call to ``step_pm_process``).

Setting ``CONFIG_STEP_PROC_MGR_POLL_RATE`` to ``0`` disables the polling thread
and leads to higher throughput, but at the cost of manually managing when
measurements are processed.

Enabling the polling thread has some additional overhead, but allows for
asynchronous measurement processing and the ability to set an appropriate
priority level for the polling thread.

Requirements
************

Depending on the device you are testing with, you may need to enable or
disable the ``CONFIG_STDOUT_CONSOLE`` flag.

It should be set to ``y`` when using the emulator, and to ``n`` when running on
real hardware.

Building and Running
********************

To run this example on the **mps2_an521 (Cortex-M33) emulator**, run the
following commands which will compile the application, run it on the emulator,
and output the result to the console:

.. code-block:: console

    $ cd samples/throughput
    $ mkdir b && cd b
    $ cmake -GNinja -DBOARD=mps2_an521 ..
    $ ninja
    $ ninja run

To do the same thing using ``west``, run:

.. code-block:: console

   $ west build -p -b mps2_an521 samples/throughput/ -t run

Press ``CTRL+A`` to exit QEMU.

To run the application on real HW, typically outputting the results to the
serial port, you can try a variant of the following, adjusting ``-DBOARD``
as appropriate.

The **LPCXpresso55S69** from NXP is used below:

.. code-block:: console

    $ cd samples/throughput
    $ mkdir build && cd build
    $ cmake -GNinja -DBOARD=lpcxpresso55s69_cpu0 ..
    $ ninja
    $ ninja flash

To do the same thing using ``west``, run:

.. code-block:: console

   $ west build -p -b lpcxpresso55s69_cpu0 samples/throughput/

Sample Output
*************

This application will normally output text resembling the following:

.. code-block:: console

   *** Booting Zephyr OS build zephyr-v2.6.0-1967-g00bfac00aecb  ***
   
   Processed 1000 measurements:
   
   total time: 162735 us
   per sample: 162 us
   mes/s:      6172
   
   Sample pool statistics:
   
   bytes_alloc (cur): 0
   bytes_alloc_total: 40000
   bytes_freed_total: 40000
   fifo_put_calls:    0
   fifo_get_calls:    0
   pool_free_calls:   1000
   pool_flush_calls:  0
   pool_alloc_calls:  1000
