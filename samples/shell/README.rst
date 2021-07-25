.. _sdp-shell-sample:

Secure Data Pipeline Shell Example
##################################

Overview
********

TODO

Requirements
************

Depending on the device you are testing with, you may need to enable or
disable the ``CONFIG_STDOUT_CONSOLE`` flag.

It should be set to ``y`` when using the emulator, and to ``n`` when running on
real hardware.

Building and Running
********************

To run this example on the **Cortex M3 emulator**, run the following commands
which will compile the application, run it on the emulator, and output
the result to the console:

.. code-block:: console

    $ cd samples/shell
    $ mkdir b && cd b
    $ cmake -GNinja -DBOARD=qemu_cortex_m3 ..
    $ ninja
    $ ninja run

To do the same thing using ``west``, run:

.. code-block:: console

   $ west build -p -b qemu_cortex_m3 samples/shell/ -t run

Press ``CTRL+A`` to exit QEMU.

To run the application on real HW, typically outputting the results to the
serial port, you can try a variant of the following, adjusting ``-DBOARD``
as appropriate.

The **LPCXpresso55S69** from NXP is used below:

.. code-block:: console

    $ cd samples/shell
    $ mkdir build && cd build
    $ cmake -GNinja -DBOARD=lpcxpresso55s69_cpu0 ..
    $ ninja
    $ ninja flash

To do the same thing using ``west`, run:

.. code-block:: console

   $ west build -p -b lpcxpresso55s69_cpu0 samples/shell/

Sample Output
*************

This application will normally output text resembling the following:

.. code-block:: console

    *** Booting Zephyr OS build zephyr-v2.6.0-536-g89212a7fbf5f  ***
    Type 'sdp help' for command options.

    1.) Populate the processor registry: sdp add
    2.) Publish measurement(s):          sdp msg
    3.) Check results:                   sdp stats

    uart:~$ sdp add
    Node #0
    -------
    Name: Temperature
    Filters: 3
    #0: IS exact match: 0x00000029 (mask 0xFFFF0000)
    #1: OR exact match: 0x00000229 (mask 0xFFFF0000)
    #2: AND exact match: 0x04000000 (mask 0xE3FFFFFF)
    Handlers:
    evaluate: no
    matched: yes
    start: yes
    run: yes
    stop: yes
    error: yes
    End of chain: no

    Node #1
    -------
    Name: Secondary temp processor
    Handlers:
    evaluate: no
    matched: yes
    start: yes
    run: yes
    stop: yes
    error: yes
    End of chain: yes

    [01:19:25.420,000] <dbg> proc_mgr.sdp_pm_register: Registering node/chain (handle 00, pri 00)
    uart:~$ sdp msg
    Published 1 measurement:
    Filter:           0x04000229
    base_type:      0x29 (41)
    ext_type:       0x02 (2)
    Flags:          0x0400
        data_format:  0
        encoding:     0
        compression:  0
        timestamp:    1
        _rsvd:        0

    Unit:             0x10000022
    si_unit:        0x0022 (34)
    scale_factor:   0x00 (10^0)
    ctype:          0x10 (16)

    SrcLen:           0x0A000008
    len:            0x0008 (8)
    fragment:       0
    _rsvd:          0
    samples:        0 (1 sample)
    sourceid:       10

    Payload: 8B F0 D0 60 00 00 00 42 
    uart:~$ sdp stats
    evaluate: 0
    matched:  1
    start:    2
    run:      2
    stop:     2
    error:    0
