#ifndef __smoke_h__
#define __smoke_h__

#include "../parameter/parameter.h"
#include "../PWM/PWM16.h"
#include "../tool/IIR.h"

class cSmoke
{
	public:
		cSmoke(cParameter * pParameter);
		void process(void);
		void enable(bool bEnable);
		bool isEnabled(void);

		uint16_t getInputVoltage_mV(void);
	
		bool isBatteryLow(void);
		
		void HWreset(void);

	private:
	
		typedef enum:uint8_t { 	smokeStateInit,
								smokeStateIdle,
								smokeStateStart,
								smokeStateActive,
								smokeStatePause,
								smokeStateEnd 
							} smokeState_et;
		
		typedef enum:uint8_t { 	outputStateOff,
								outputStateStart,
								outputStateDelay,
								outputStateUp,
								outputStateSmoke,
								outputStateDown,
								outputStateEnd
							} outputState_et;
		
		typedef enum:uint8_t {	paramOutputDelay,
								paramOutputUpPower,
								paramOutputUpDuration,
								paramOutputSmokePower,
								paramOutputDownPower,
								paramOutputDownDuration,
								paramOutputLimit,
								paramOutputEnd} paramOutput_et; 

		typedef struct outputState_s {	outputState_et 	state;
										uint16_t		timer;
									} outputState_st;

		typedef struct cycleState_s {	uint16_t		timer;
										uint16_t		counter;
										uint16_t		upTime;
										uint16_t		downTime;
									} cycleState_st;
									
		typedef struct inputVoltageState_s {	
										uint16_t		voltageTimer;
										uint16_t		batteryLowCount;
										uint16_t		batteryLimitADCVal;
										IIR<uint16_t,uint16_t> inputVoltageADC{filterBits};
									} inputVoltageState_st;
		
		typedef struct smokeState_s {	smokeState_et			smokeState;
										outputState_st			airState;
										outputState_st			heatState;
										cycleState_st			cycleState;
										inputVoltageState_st	inputVoltageState;
										uint16_t				sysTime;
										uint16_t				lastTime;
									} smokeState_st;

		static const cParameter::valID_et airParamterID[paramOutputEnd] PROGMEM;
		static const cParameter::valID_et heatParamterID[paramOutputEnd] PROGMEM;

		static const cPWM16::pwmPin_et airPin=cPWM16::PWM_A;	// air pump is on OC1A (Pin 9)
		static const cPWM16::pwmPin_et heatPin=cPWM16::PWM_B;	// heat coil is on CO1B (Pin 10)
		static const uint8_t	inputVoltageADCPin=0;
		static const uint8_t	filterBits=5;
		static const uint16_t	timerValVoltage=100;			// read voltage every 100ms
		static const uint16_t	Rscale=23;						// Voltage divider is R1=20k R2=0,91k ~1:23
		                                                        // max 25,3V (25,276V) <0,1% Error
		static const uint16_t	batteryLimitCount=50;			// battery voltage needs to be loww for 50 cycles 
		static const uint16_t	referenceADCmV=1100;			// ADC reference Voltage
		static const uint8_t	ADCbits=10;						// ADC resolution is 10 bit
		static const uint8_t 	resetOutPin=17;					// reset Out is D17 (A3)
		cParameter * pParameter;
		cPWM16		PWM;
		
		smokeState_st	smokeState;
		
		void processCycle(void);

		void processOutput(	outputState_st & outputState, 		// state variable
							const cPWM16::pwmPin_et outPin,		// output pin
							const cParameter::valID_et* idList);// list with val ID

		bool checkTimer(uint16_t & timer, uint16_t time);
		
		void setTimer(uint16_t & timer, uint16_t timeVal);

		void setCycleStart(void);
		
		uint16_t readInputVoltageADC(void);

		uint16_t getOutputParameter(const cParameter::valID_et * idList, 
									paramOutput_et paramOutput); 
		uint16_t calcPowerSetting(	const cParameter::valID_et * idList, 
									paramOutput_et paramOutput);
									
		void calcBatteryLowLimit(void);

		void checkInputVoltage(void);
};

#endif //__smoke_h__
