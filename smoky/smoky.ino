#include "src/button/button.h"
#include "src/PWM/PWM16.h"
#include "src/parameter/parameter.h"
#include "src/UI/menu.h"


/* *************************************************************************************
 * smokey - the smoke machine
 * smokey is based on a 5V fish tank air pump and a 1S vape head
 * the HW is designed to work of 5 V to 15 V (or above) input voltage
 * Base line operation can be set by the air and heating limit
 * ************************************************************************************ */


cMenu	menu;

uint16_t pwm1;

void setup() {
  // put your setup code here, to run once:
	
}




void loop() {
  // put your main code here, to run repeatedly:
  menu.process();
}

