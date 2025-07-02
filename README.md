# EPOS4_PID_wDoB
This repository contains source code to run a motor using an EPOS4 controller in current control mode using PID with disturbance compensation
- This is a code developed by editing the demo code provided by maxongroup for their EPOS4 motor controller
- The code has been modified for maxon EC 90 Flat BLDC motor, but can be easily customised for any other EPOS4 compatible motor
- The code can only be used to run one motor simultaneously using a EPOS4 controller connected to the PC via USB
- The motor parameters are specific to EC 90 Flat, therefore must be changed for any other motor according to datasheet
- To find friction parameters, the user might have to run a system identification test
- The code runs the motors in CurrentMode, where a target current is specified by the user. Before running the code, 
the PID gains for the motor must be set to (Kp = 1, Ki = 0, Kd = 0) using the EPOS Studio (see EPOS4 manual for more informaion: 
https://www.maxongroup.com/medias/sys_master/root/8837359304734/EPOS4-Application-Notes-Collection-En.pdf)
