#ifndef __PWM16_H__
#define __PWM16_H__
#include <stdint.h>

/* **********************************************
 * class for 16 bit PWM on Timer 1 on ATmega328P
 * uses only fast 10bit PWM on OC1A and OC1B at
 * 
 * OC1A Pin is digital Pin 9 or PortB.1
 * OC1B Pin is digital Pin 10 or PortB.2
 * 
 * maximum timer frequency
 * 	resulting in 16MHz/1024=15625Hz
 * 	
 * 
 * 
 */
 
 
class cPWM16
{
	public:
		typedef enum:uint8_t { PWM_A, PWM_B } pwmPin_et;
		cPWM16(void);
		void begin(void);								// initializes Timer
		void attach(pwmPin_et pin);						// enables PWM on pin
		void release(pwmPin_et pin);					// disables PWM on pin
		void setPeriod(pwmPin_et pin, uint16_t period);	// sets PWM Period 0-1023
	
	private:
		typedef struct flag_s { uint8_t	initialized	:1;			// begin called
								uint8_t	attached_A	:1;			// attach/release called for OC1A
								uint8_t	attached_B	:1;			// attach/release called for OC1B
								uint8_t	DDR_A		:1;			// previous DDR state for OC1A Pin
								uint8_t	DDR_B	    :1;			// previous DDR state for OC1B Pin
								uint8_t	PORT_A      :1;			// previous PORT state for OC1A Pin
								uint8_t	PORT_B      :1;			// previous PORT state for OC1B Pin
							} flag_st;
							
		flag_st flags;
				
		void save_pinmode(pwmPin_et pin);
		void restore_pinmode(pwmPin_et pin);
};	

extern cPWM16 PWM16;

#endif // __PWM16_H__
