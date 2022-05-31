.. step-fusion-sample:

Secure Telemetry Pipeline (STeP) Sensor Fusion Example
######################################################
This sample presents a Inertial Measurement Unit, IMU
providing data to STeP and a pipeline is created to
perform and simple sensor fusion processing node.

Overview
********
The sample collects, at 100Hz data from the on-board IMU,
the accelerometer plus the gyroscope, then this data feeds the
STeP fusion pipeline that executes a sensor fusion algorithm based
on the Mahoney filtering delivering the Roll and Pitch Euler angles
in degrees, the Yaw angle is not provided since it depends on magnetometer
and a proper calibration procedure which is not currently supported.

The Orientation delivered can be evaluated in the Zephyr's console
using the step_fusion subcommands, just type it in terminal to see the
supported commands, it is also possible to tune the mahoney filtering 
proportional and integral constants based on the IMU used using the
same set of offered commands.

Following the presentation the data using the console, the STeP also
executes a streaming node that continuosly send the IMU data over the
serial port to be evaluated graphcally.

Requirements
************
If user wants to see the streamed data, it needs to attach the board specific
serial port pins to a Serial-to-USB converter to capture the straming data
sent by the board.

Building and Flashing
*********************
Using this sample with the supported ST U585 IoT Discovery board does not 
require special instructions, just use west command as shown below:

.. code-block:: console

    west build  -p always -b b_u585i_iot02a  modules/lib/step/samples/fusion_imu

Flashing also does not require special steps, just type:

.. code-block:: console

    west flash

Expected result
***************
After connecting the board to the console you should wait the calibration
of the gyroscope sensor and fusion driver initialization:

.. code-block:: console

    uart:~$ *** Booting Zephyr OS build zephyr-v3.0.0-3946-ga70ff60c4b91  ***
    [00:00:00.001,000] <inf> ISM330DHCX: chip id 0x6b
    [00:00:00.011,000] <dbg> proc_mgr: step_pm_register: Registering node/chain (handle 0, pri 0)
    [00:00:00.014,000] <inf> imu_data_producer: Calibrating gyro, please place the board in flat surface and wait up 5 seconds!
    [00:00:02.156,000] <inf> imu_data_producer: Gyro offset correction vector: 0.006822, 0.000069, 0.006278

Then it is possible to evaluate the euler angles using the shell commands:

.. code-block:: console

    uart:~$ step_fusion euler
    Euler angles x =  -2.157234[d] y =  -2.365983[d]
    uart:~$ step_fusion euler
    Euler angles x =  -7.522989[d] y = -80.391792[d]

For seeing the data streamed, user may use the BetterSerialMonitor, a cross platform
serial monitor. The links for downloading the package can be found below, as the 
its source code: 

* The project: https://hackaday.io/project/181686-better-serial-plotter/details
* The source code: https://github.com/nathandunk/BetterSerialPlotter

After tunning the fusion algorithm, just type the command stream from step_fusion
command sets to start the data streaming over the console, this command takes an extra
argument which is the streaming data in seconds.

Mahoney Tunning
***************
Fusion sensor algorithm in general require tunning for a particular
IMU set, depending on the acquisition sample rate, noise, bias, etc.
The Mahoney filter offers tunning on its proportinal-integral controller
to make the observer error quickly converging near to zero.

The Mahoney filter can be tuned by changin the kp (proportional gain), 
and ki (the integral gain), both values have no scale, and use PID 
controllers tunning method like Ziegler-Nichols for example.

For this sample, a manual tunning empirically based was done which 
can be used as starting point.

Align the board to one of the axis, and increase kp until getting the
output near of 90 degrees without oscilation, the kp here plays a role
to proper scale the observer results near to the reality but keeping a 
offset when the PI controller enters in steady state, once getting a reasonable
kp in this condition, start to increment the ki in a fraction of kp, to increase
the response time and reduce the offset near to a reasonable result. for ki 
consider it as a function of kp and number of sample periods: ki = kp / n, where
"n" represent the numer of sample periods to integrate over.

Larger periods of integration leads to lower oscilations and reduces the drift
over time, which means smaller ki. Higher ki reduces the offset but may introduce
oscilation plus drifting over the time.

For tunning at runtime, invoke commands set plus calibrate in the step_fusion command
set.
