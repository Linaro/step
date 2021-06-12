.. _sensor_pipeline:

Secure Sensor Pipeline for Zephyr
#################################

Overview
********

An experimental data processing pipeline that can be used with any
:ref:`supported board <boards>`.

Building and Running
********************

This application can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: projects/sensor_pipeline
   :host-os: unix
   :board: qemu_x86
   :goals: run
   :compact:

To build for another board, change "qemu_x86" above to that board's name.

Sample Output
=============

.. code-block:: console

    TODO

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.
