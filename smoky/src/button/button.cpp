#include "button.h"
#include <Arduino.h>
#include <PinChangeInterrupt.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterruptPins.h>
#include <PinChangeInterruptSettings.h>

uint8_t button_c::instanceCnt=0;
button_c * button_c::pInstance0;
button_c * button_c::pInstance1;
button_c * button_c::pInstance2;
button_c * button_c::pInstance3;
button_c * button_c::pInstance4;


button_c::button_c(uint8_t pin, bool normal_open, bool pullup)
{	// button_c
	switch_pin=pin;
	/// Switch polarity true = active high
	switch_polarity=(normal_open && !pullup) || 
					(!normal_open && pullup);
	pull_up=pullup;
	
	this->instance=instanceCnt;
	instanceCnt++;
}	// button_c

void button_c::init(void)
{	// init
	// configure pin & interrupts
	pinMode(switch_pin,(pull_up)?(INPUT_PULLUP):(INPUT/*_PULLDOWN*/));
	button_short=false;      
	button_released=false;
	button_state=pin_state();	
	button_time=millis();	
	switch(instance)
	{
		case 0:
			attachPCINT(digitalPinToPCINT(switch_pin), handler0, CHANGE);	
			pInstance0= this;
			break;
		case 1:
			attachPCINT(digitalPinToPCINT(switch_pin), handler1, CHANGE);	
			pInstance1= this;
			break;
		case 2:
			attachPCINT(digitalPinToPCINT(switch_pin), handler2, CHANGE);	
			pInstance2= this;
			break;
		case 3:
			attachPCINT(digitalPinToPCINT(switch_pin), handler3, CHANGE);	
			pInstance3= this;
			break;
		case 4:
			attachPCINT(digitalPinToPCINT(switch_pin), handler4, CHANGE);	
			pInstance4= this;
			break;
		default:
			break;
	}
}	// init


bool button_c::pressed(void)
{
	if (button_short)
	{
		button_short=false;
		button_released=false;
		return true;
	}
	else
		return false;
}

bool button_c::released(void)
{

	if (button_released)
	{
		button_released=false;
		return true;
	}
	else
		return false;
}

bool button_c::long_press(void)
{
	return 	(button_state) && 
			((uint16_t)(millis()-button_time)>LONG_PRESS_MIN);
}

bool button_c::down(void)
{
	return button_state;
}

bool button_c::pin_state(void)
{
	return (digitalRead(switch_pin)==switch_polarity);
}

void button_c::button_handler(void)
{
	bool 		tempState=pin_state();
	uint16_t	tempTime=millis();
	if (button_state!=tempState)
	{	// change, source was pin
		if (button_state && !tempState)
		{ // button has been released
			button_short=	((uint16_t)(tempTime-button_time)<LONG_PRESS_MIN) &&
							((uint16_t)(tempTime-button_time)>PRESS_MIN);
			button_released=true;
		} // button has been released
		else
			button_released=button_released && !tempState;
		button_state=tempState;
		button_time=tempTime;
	}	// change, source was pin
}

void button_c::handler0(void)
{	// handler0
	pInstance0->button_handler();
}	// handler0
void button_c::handler1(void)
{	// handler1
	pInstance1->button_handler();
}	// handler1
void button_c::handler2(void)
{	// handler2
	pInstance2->button_handler();
}	// handler2
void button_c::handler3(void)
{	// handler3
	pInstance3->button_handler();
}	// handler3
void button_c::handler4(void)
{	// handler4
	pInstance4->button_handler();
}	// handler4
