#include "PWM16.h"
#include <Arduino.h>

#define PWM_PIN_A_MSK _BV(1)
#define PWM_PIN_B_MSK _BV(2)
#define PWM_DDR_A DDRB
#define PWM_DDR_B DDRB
#define PWM_PORT_A PORTB
#define PWM_PORT_B PORTB

cPWM16 PWM16;

/* ************************************************
 * public
 */

 
cPWM16::cPWM16(void)
{
	flags.initialized	=false;
	flags.attached_A	=false;
	flags.attached_B	=false;
	flags.DDR_A			=false;
	flags.DDR_B			=false;
	flags.PORT_A		=false;
	flags.PORT_B		=false;
}								

void cPWM16::begin(void)								// initializes Timer
{	// begin
	// initialize Timer
	if (!flags.initialized)
	{	// not jet done
		flags.initialized=true;
		
		// set timer controll WGM13:10 to 0b0111 
		// FastPWM 10bit
		// TOP = 0x3FF
		// update OCR1x on BOTTOM
		// TOV1 Flag on TOP
		// Clock auf 16MHz
		TCCR1A = 1 << WGM11 | 1 << WGM10;	
		TCCR1B = 1 << WGM12 | 1 << WGM12 |	1 << CS10;					 
	}	// not jet done
}	// begin

void cPWM16::attach(pwmPin_et pin)						// enables PWM on pin
{	// attach
	switch (pin)
	{	// switch Pin
		case PWM_A:
			if (!flags.attached_A)
			{	// not attached
				flags.attached_A=true;
				
				// prepare pin
				save_pinmode(PWM_A);

				// initial off
				setPeriod(PWM_A,0);
				
				// enable PWM Mode 10 => FAST PWM non Inverting
				TCCR1A=(TCCR1A & (~(1 << COM1A1 | 1 << COM1A0))) | 1 << COM1A1; 
			}	// not attached
			break;
		case PWM_B:
			if (!flags.attached_B)
			{	// not attached
				flags.attached_B=true;
				
				// prepare pin
				save_pinmode(PWM_B);

				// initial off
				setPeriod(PWM_B,0);
				
				// enable PWM Mode 10 => FAST PWM non Inverting
				TCCR1A=(TCCR1A & (~(1 << COM1B1 | 1 << COM1B0))) | 1 << COM1B1; 
			}	// not attached
			break;
		default:
			break;
	}	// switch Pin
}	// attach

void cPWM16::release(pwmPin_et pin)						// disables PWM on pin
{	// release
	switch (pin)
	{	// switch Pin
		case PWM_A:
			if (flags.attached_A)
			{	// attached
				flags.attached_A=false;

				// disable pwm
				TCCR1A=(TCCR1A & (~(1 << COM1A1 | 1 << COM1A0))); 

				// restore pin
				restore_pinmode(PWM_A);
			}	// attached
			break;
		case PWM_B:
			if (flags.attached_B)
			{	// attached
				flags.attached_B=false;

				// restore pin
				restore_pinmode(PWM_B);
				// disable pwm
				TCCR1B=(TCCR1B & (~(1 << COM1B1 | 1 << COM1B0))); 
			}	// attached
			break;
		default:
			break;
	}	// switch Pin
}	// release

void cPWM16::setPeriod(pwmPin_et pin, uint16_t period)	// sets PWM Period 0-1023
{	// setPeriod
	period&=0x3FF;
	switch (pin)
	{	// switch Pin
		case PWM_A:
			OCR1A=period;
			break;
		case PWM_B:
			OCR1B=period;
			break;
		default:
			break;
	}	// switch Pin
}	// setPeriod


/* *************************************************
 * private
 */

void cPWM16::save_pinmode(pwmPin_et pin)

{	// save_pinmode
	switch (pin)
	{	// switch pin
		case PWM_A:
			// save settings
			flags.DDR_A	=PWM_DDR_A 	& PWM_PIN_A_MSK;
			flags.PORT_A=PWM_PORT_A & PWM_PIN_A_MSK;
			// set to output and 0
			PWM_PORT_A	&= ~PWM_PIN_A_MSK;
			PWM_DDR_A	|= PWM_PIN_A_MSK;
			break;
		case PWM_B:
			// save settings
			flags.DDR_B	=PWM_DDR_B 	& PWM_PIN_B_MSK;
			flags.PORT_B=PWM_PORT_B & PWM_PIN_B_MSK;
			// set to output and 0
			PWM_PORT_B	&= ~PWM_PIN_B_MSK;
			PWM_DDR_B	|= PWM_PIN_B_MSK;
			break;
		default:
			break;
	}	// switch pin
}	// save_pinmode

void cPWM16::restore_pinmode(pwmPin_et pin)
{	// restore_pinmode
	switch (pin)
	{	// switch pin
		case PWM_A:
			if (flags.PORT_A)
				PWM_PORT_A	|= PWM_PIN_A_MSK;
			else
				PWM_PORT_A	&= ~PWM_PIN_A_MSK;
			if (flags.DDR_A)
				PWM_DDR_A	|= PWM_PIN_A_MSK;
			else
				PWM_DDR_A	&= ~PWM_PIN_A_MSK;
			break;
		case PWM_B:
			if (flags.PORT_B)
				PWM_PORT_B	|= PWM_PIN_B_MSK;
			else
				PWM_PORT_B	&= ~PWM_PIN_B_MSK;
			if (flags.DDR_B)
				PWM_DDR_B	|= PWM_PIN_B_MSK;
			else
				PWM_DDR_B	&= ~PWM_PIN_B_MSK;
			break;
		default:
			break;
	}	// switch pin
}	// restore_pinmode


#undef PWM_PIN_A_MSK 
#undef PWM_PIN_B_MSK 
#undef PWM_DDR_A 
#undef PWM_DDR_B 
#undef PWM_PORT_A
#undef PWM_PORT_B
