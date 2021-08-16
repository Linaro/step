.. step-shell-sample:

Secure Telemetry Pipeline (STeP) Shell Example
##############################################

Overview
********

This sample exposes a ``step`` shell command that can be used to test common
featurs of STeP.

A simple workflow is shown below:

1. Add some nodes to the processor registry

.. code-block:: console

   uart:~$ step add
   [00:00:38.665,241] <dbg> proc_mgr: step_pm_register: Registering node/chain (handle 0, pri 4)
   [00:00:38.665,328] <dbg> step_shell: step_shell_cmd_add: Took 93880 ns

2. Display the registry contents

.. code-block:: console

   uart:~$ step list
   Processor node registry:
   0 (priority 4):
     Total processing time: 0 ns (0 avg, 0 runs)
     [0] Root processor node
     [1] 2nd processor node
     [2] 3rd processor node

3. Publish a measurement to the processor node manager

.. code-block:: console

   uart:~$ step pub
   [00:00:08.390,434] <dbg> step_shell: step_shell_cmd_pub: Published 1 measurement
   [00:00:08.390,483] <dbg> step_shell: step_shell_cmd_pub: Took 96640 ns, excluding polling thread processing time (run 'step list').
   [00:00:08.499,856] <inf> step_shell: [47253030] Received die temp: 32.00 C (handle 0:0)
   [00:00:08.499,904] <inf> step_shell: cfg: mult by 10.00 (handle 0:1)
   [00:00:08.499,947] <inf> step_shell: [47253030] Received die temp: 320.00 C (handle 0:1)
   [00:00:08.499,997] <inf> step_shell: [47253030] Received die temp: 320.00 C (handle 0:2)

4. Show stats for the registered processor nodes

.. code-block:: console

   uart:~$ step stats
   init:     3
   evaluate: 0
   matched:  1
   start:    1
   run:      3
   stop:     1
   error:    0

5. Look at the sample pools stats

.. code-block:: console

   uart:~$ step pool
   bytes_alloc (cur): 0
   bytes_alloc_total: 32
   bytes_freed_total: 32
   fifo_put_calls:    1
   fifo_get_calls:    157
   pool_free_calls:   1
   pool_flush_calls:  0
   pool_alloc_calls:  1

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

    $ cd samples/shell
    $ mkdir b && cd b
    $ cmake -GNinja -DBOARD=mps2_an521 ..
    $ ninja
    $ ninja run

To do the same thing using ``west``, run:

.. code-block:: console

   $ west build -p -b mps2_an521 samples/shell/ -t run

Press ``CTRL+A`` to exit QEMU.

To run the application on real hardware, typically outputting the results to the
serial port, you can try a variant of the following, adjusting ``-DBOARD``
as appropriate.

The **LPCXpresso55S69** from NXP is used below:

.. code-block:: console

    $ cd samples/shell
    $ mkdir build && cd build
    $ cmake -GNinja -DBOARD=lpcxpresso55s69_cpu0 ..
    $ ninja
    $ ninja flash

To do the same thing using ``west``, run:

.. code-block:: console

   $ west build -p -b lpcxpresso55s69_cpu0 samples/shell/

Sample Output
*************

.. code-block:: console

    *** Booting Zephyr OS build zephyr-v2.6.0-536-g89212a7fbf5f  ***
    Type 'step help' for command options.

    1.) Populate the processor registry: step add
    2.) Publish measurement(s):          step pub
    3.) Check results:                   step stats

    uart:~$
