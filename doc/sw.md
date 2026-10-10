# the software


##### Table of contents
- [genral](#general)
- [modules](#modules)
	- [smoky](#smoky)
	- [ASCII](#ASCII)
	- [button](#button)
	- [parameter](#paramter)
	- [PWM](#PWM)
	- [smoke](#smoke)
	- [tool](#tool)
		- [convert](#convert)
		- [IIR](#IIR)
		- [version](#version)
	- [UI](#UI)
		- [icondef](#icondef)
		- [menudefs](#menudefs)
		- [menu](#menu)
- [home](../README.md#Table-of-contents)

## general
I used the Arduino IDE to built the SW.  
I used two external libraries:
- the Adrafruit Grafix Libraries for the SSD1306.  
- the [PinChangeInterrup](https://github.com/NicoHood/PinChangeInterrupt) library from Nico Hood.
The PWM is hardcoded and should only work on ATmega 328 and compatible chips.  

## modules

### smoky
The main module smoky.ino is just used as start pointand main sceduler for the menu process  
### ASCII
The files ASCII_box and ASCII_ctrl do contain definitions for ASCII control characters 
0x00-0x1f and 0x7f as well as definitions for the IBM-ASCII table characters (horizontal 
and vertical lines, corners etc)  
### button
This module implements a simple button detection with debouncing and long / short press 
detection. There can be up to 5 instances on different pins of this module.
### parameter
The parameter module uses a modified TLV mechanism on the internal EEPROM. Because there are 
only 16 bit values, the Length item is omitted from the record and only 3 bytes are written.  
### PWM
This modul uses the 16 bit PWM on Timer 1. It is fixed to fast 10bit PWM on OC1A and OC1B.
### smoke
This module handles the output pins and the output state machine
### tool
I use three small tool modules
#### convert
This module is used to convert numbers, intergers and fixed point (IQ) numbers to strings
#### IIR
This is a template to create a simple IIR filter: X(n)=[X(n-1)*(N-1)+X]/N, where N is 2^q, 
where q is a positive interger  
#### version
just a Version string and built date  
### UI
anything associated with the userinterface  
#### icondefs
bitmap definitions of menu icons  
#### menudefs
data structures for menu structure and navigation and endpoints
#### menu
the menu module displays the menu according to data from nebudefs and icondefs. 


