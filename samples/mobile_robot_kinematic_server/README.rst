.. step-fusion-sample:

Secure Telemetry Pipeline (STeP) Multicore example
######################################################

Overview
********
This sample shows two STeP physical instances running in each
core of a, multicore capable, supported board, this example gives
each core an responsibillity, the master core receives a set of 
four motor rotational speeds in RPM and performs the robot body velocity
estimation, which is called direct Kinematic, at the same time this output
calculation is sent to the secondary core which calculates back the 
motor rotational speeds from the desired robot body velocity and reports it
back to the primary core, the results are then printed.


Building and Flashing
*********************
Example below shows how to build this sample to mps2_an521, which is multicore cabaple:

.. code-block:: console

    west build  -p always -b mps2_an521  modules/lib/step/samples/mobile_robot_kinematic_server

It is also possible to run in the qemu:

.. code-block:: console

    west build -t run

Expected result
***************
After connecting the board to the console you should be able to use commands
to perform the kinematics calculation, in a real world robotic application the 
commands given would be the output of a high-level subsystem like the robotic 
navigation stack output, the commands are:

.. code-block:: console

    *** Booting Zephyr OS build zephyr-v3.1.0-2488-gee4919d1a3ed  ***

    <dbg> proc_mgr: step_pm_register: Registering node/chain (handle 0, pri 0)
    uart:~$ step_robot help
    step_robot - Secure telemetry pipeline Mobile Robot Kinematics commands
    Subcommands:
    kinematics  :Set robot base kinematics: Width[mm], Height[mm], and Wheel
                Radius[mm]
    set_speeds  :Set the four joints speeds[rpm] to obtain Kineamtics calculation
    calculate   :Perform calculation with given parameters

The kinematics command is used to set the robot base dimensions, for this application
a robot base with four wheels is considered, the shape should always be treated as a 
rectangle one. All the dimentions should be passed in milimeters.

After setting the mobile robot base the set_speeds command waits for four rotational 
speeds in each robotic base motor, in RPM, these speeds are the desired one which will
produce the robotic body velocity vector which is utilized to perform its trajectory build.

Once speeds are set, user should use the calculate command, no parameters are required, this
commmand will pick the speed set and will start the calculation process into the two cores,
the primary one will do the Direct Kineamtics, STeP allocates a sample, fill with the speed sets,
and triggers the processing node, resulting to obtain the robotic velocity vector
from the joint speed sets, the result is sent to the remote core using the openAMP through 
RPMsg protocol, the secondary core, which also runs STeP, receives this data and in the same way
STeP instance allocates a sample, fills, and triggers the Inverse Kinematic processing node, 
resulting on the desired speeds into each motor from the robotic base, the result is then sent
back to the primary core, through RPMsg and the comparisons are printed at the console:

.. code-block:: console

    uart:~$ step_robot kinematics 1 1 1 
    Kinematics set Height: 1.000000 [mm], Width: 1.000000 [mm], 6.283200 Wheel Radius Scaled [mm] 
    uart:~$ step_robot set_speeds 1 1 1 1
    Speed joint commmand vector: [ 1.000000, 1.000000, 1.000000, 1.000000] 
    uart:~$ step_robot calculate 
    rpmsg registered endpoint number: 0
    *** Booting Zephyr OS build zephyr-v3.1.0-2488-gee4919d1a3ed  ***
    ========= Calculated Kinematics for the Robot base: ============ 
    Base length: 1.000000 [mm] 
    Base height: 1.000000 [mm] 
    Wheel Radius: 6.283200 [mm] 
    ========= Body Velocity from Direct Kinematics:  ============ 
    Desired RPMs: 1.000000, 1.000000, 1.000000, 1.000000 [rad/min] 
    Estimated Body Velocity[x,y, heading]: 0.104720, 0.000000, 0.000000
    ========= Joint Speeds from Inverse Kinematics:  ============ 
    Estimated Body Velocity in Local Core [x,y, heading]: 0.104720, 0.000000, 0.000000 
    Output joint RPMs from Remote Core: 1.000000, 1.000000, 1.000000, 1.000000 [rad/min] 
    =====================================================================
    uart:~$ 

Mobile Robot body frame considered:
***********************************
The image below shows the simplified 2-D view from the mobile robot body frame
which is used as reference:

.. raw:: html

   <p align="center">
     <img src="img/robotframe.png" align="center" alt="Reference of the Mobile Robot Body">
   </p>