# PhoneWave-DisplayUnit
Code for the ATmega8 to control the PhoneWave-Projects with 4-bit input given on Ports C0-3 which are read when the interrupt has been triggered by a rising edge signal on Port D2

This code is rather old. It has been designed to convert the output of the MT8870 IC (which translates DTMF signals into binary numbers) into human-readable digits displayed on a 7-segment display (common anode JLDF-4431BHGBD20 930 display) provided by my microwave manufacturer.  

It outputs the 7-segment-display-code on Port B and controls the multiplexing of the digits on Port D 2-6

Included are also the schematics and the design of the self-built boards.   .LM4 files are to be openend with LochMaster 4 whilst the .pro  and .sch files are to be opened with KiCad.  
The extension Board is a "shield" to connect the Raspberry Pi with the Microwave and ATmega8. The "relay microwave" board connects the ATmega8 to the 230V lightbulb and turntable motor.

All files with legacy in their name still use the MT8870 IC, whilst thise without legacy in their name rely on the data given by my Raspberry Pi. Remember to use a Logic Level Shifter between the ATmega and the Raspberry Pi!  
