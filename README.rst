.. _sensor_pipeline:

Secure Data Pipeline (SDP) for Zephyr
#####################################

Overview
********

An experimental data processing pipeline that can be used with any
:ref:`supported board <boards>`.

Adding sdp to your project via ``west``
***************************************

For projects that have been setup using ``west``, you can add a local copy of
this module by adding the following sections to ``zephyr/west.yml``:

1. In the ``manifest/remotes`` section add:

.. code-block::

   remotes:
     - name: microbuilder
       url-base: https://github.com/microbuilder

2. In the ``manifest/projects`` section add:

.. code-block::

   - name: linaro_sensor_pipeline
     remote: microbuilder
     path: modules/lib/sdp
     revision: main

3. Save the file, and run ``west update`` from the project root to retrieve the
latest version of the library from Github, or whatever ``revision`` was
specified above.

Building and Running
********************

This application can be built and executed on QEMU as follows:

.. code-block:: console

   $ west build -p auto -b qemu_cortex_m3 modules/lib/sdp/samples/shell/ -t run

To build for another board, change ``qemu_cortex_m3`` above to that board's
name, and remove the ``-t run`` appendix.

Sample Output
=============

.. code-block:: console

   TODO

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.

Running Unit Tests
==================

To run the unit tests for this module, you can run ``twister`` via:

.. code-block:: console

   $ cd $ZEPHYR_BASE
   $ twister --inline-logs \
     -p qemu_cortex_m3 \
     -T ../modules/lib/sdp/tests

Basic Architecture
******************

The Secure Data Pipeline (SDP) aims to implement an extensible workflow to
process generic sensor data (**measurements**) in a content-agnostic manner.

In theory, any type of measurement, using any standard SI unit and scale, and
represented in any standard C type should be expressable in a relatively
light-weight manner, keeping in mind the memory constraints of small embedded
systems.

Processing of measurements happens based on one or more **processor nodes**,
which can be chained together for more complex operations.

Processor nodes have an optional filter mechanism to indicate which types of
measurements they process, allowing for processing workflows to be defined on
a per-measurement-type basis.

The **Secure** in SDP comes from the goal to provide basic secure processor
nodes out of the box, implementing common operations like: hash, sign,
compress, encrypt, etc.

A high-level overview of the system is shown here:

.. raw:: html

   <p align="center">
     <img src="doc/Overview.png" align="center" alt="Basic Workflow">
   </p>

Measurement Values
==================

Measurements are the main component in SDP, and traverse the system starting
as inputs from a data source, are processed, and output to an appropriate
data sink(s).

SDP attempts to compromise between optimising for memory in small embedded
systems, and trying to describe exactly what this measurement represents in as
expressive a manner as possible. It aims to balance the ability to precisely
represent the exact meaning of the measurement, without wasting precious memory
on that representation.

Measurements make use of the following header, with a 12-byte overhead:

::

      3                   2                   1
    1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |              Flags            |  Ext. M Type  |  Base M Type  | <- Filter
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     C Type    | Scale Factor  |         SI Unit Type          | <- Unit
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Source ID   | S Cnt | R | F |        Payload Length         | <- SrcLen
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                            Payload                            |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                      Timestamp (optional)                     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
              1
    5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | Res | TSt | CMP | Encod |  DF | <- Flags
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |    |     |      |      |
       |    |     |      |      +-------- Data Format (CBOR, etc.)
       |    |     |      +--------------- Encoding (BASE64, BASE45, etc.)
       |    |     +---------------------- Compression (LZ4, etc.)
       |    +---------------------------- Timestamp
       +--------------------------------- Reserved (version flag?)

For futher technical details, see ``ìnclude/sdp/measurement.h``, but a
high-level summary of these three key words is shown below:

Filter
------

The **Filter** word allows processor nodes to determine if this measurement
interests them or not.

It consists of an 8-bit **Base Measurement Type**, and an optional 8-bit
**Extended Measurement Type**, which can be used to specialise the meaning of
the base type.

EXAMPLE: ``SDP_MES_TYPE_LIGHT`` is a base type, which uses a default
SI unit of ``SDP_MES_UNIT_SI_LUX``. If we wish to represent a different
measurement in the same measurement family (base type), we could indicate
``SDP_MES_EXT_TYPE_LIGHT_RADIO_RADIANCE`` as the extended type, which
represents a radiometric measurement based on W/(sr m^2).

The **Flags** field indicates other important data about this measurement
packet, such as how the data has been formatted, encoded, what compression
algorithm has been used (if any), and if a timestamp is present.

Unit
----

The **Unit** word describes the SI unit and optional scale factor this
measurement uses, as well as how that unit is represented in memory. A 32-bit
floating-point value may use less memory in most cases, but we may require the
additional range and precision a 64-bit float provides. The ``unit`` word
allows for a flexible expression of this information on a per-measurement basis,
without an excessive amount of overhead.

Standard SI units, scale factors and C types are all represented via enums in
SDP in the ``include/sdp/measurement`` folder.

SrcLen
------

The **Source/Len** word describes the size of the payload, with an option to
spread larger payloads over multiple packets.

It also indicates the number of samples present in this measurement payload,
in steps of power of two (2, 4, 8, 16, 32, etc., samples). This allows for
better use of system resources by hashing, signing and encrypting larger sets
of data, with only one 12-byte header as additional memory overhead. The 4-bits
reserved to indicate that multiple samples are present allows for between 2 and
32768 samples to be stored in the payload (2^n):

::

   0 = 1 sample (default)     8 = 256 samples
   1 = 2 samples              9 = 512 samples
   2 = 4 samples              10 = 1024 samples
   3 = 8 sammples             11 = 2048 samples
   4 = 16 samples             12 = 4096 samples
   5 = 32 samples             13 = 8192 samples
   6 = 64 samples             14 = 16384 samples
   7 = 128 samples            15 = 32768 samples

It also contains an 8-bit **Source ID** field, which allows the measurement
value's source to be identified to retrieve further information about the
source device, such as it's min/max values, sample rate, gain setting, etc.

Measurement Memory Management
=============================

In order to minimize endless memcpy operations, and deal with variable length
measurements, all ``sdp_measurement`` records are allocated from a central
heap memory block managed by the **sample pool manager**.

Allocating and freeing memory imposes a certain amount of rigor on behalf of
the developper, and heap memory fragmentation may be an issue over time, but
at present this seems like the best tradoff for an initial proof of concept.

The allocation, population, consumption and release of the measurement packet
is describe in the sequence diagram below:

.. raw:: html

   <p align="center">
     <img src="doc/SamplePool.png" align="center" alt="Sample Pool Memory Management">
   </p>

Filter Engine
=============

The **processor manager** makes uses of the ``.filter`` word in measurements to
optionally determine if registered filter nodes should or shouldn't process
the incoming measurement value(s). 

If the processor node's filter chain is set to ``NULL`` (default), it will
accept all incoming measurements. If one or more filters are indicated for the
processor node, the filter engine will evaluate the measurement's filter fields
against the processor node's filter value(s), to determine if there is a match.

This evaluation process introduces some overhead, which can be addressed by
enabling **filter caching**, which works as follows:

.. raw:: html

   <p align="center">
     <img src="doc/FilterEngineCache.png" align="center" alt="Filter Engine Caching">
   </p>
