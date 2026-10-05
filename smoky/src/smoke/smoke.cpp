#include <Arduino.h>
#include "smoke.h"

// for Test without power supply (5V USB only) enable TEST-Makro
// this will simulate 9V input and enable Data output on smoke start
//#define TEST_5V_USB

static const cParameter::valID_et cSmoke::airParamterID[paramOutputEnd] PROGMEM = {
																					cParameter::valID_airStartDelay,	//	paramOutputDelay,	
																					cParameter::valID_airStartPower,	//	paramOutputUpPower,	
																					cParameter::valID_airStartDuration,	//	paramOutputUpDuration,	
																					cParameter::valID_airSmokePower,	//	paramOutputSmokePower,	
																					cParameter::valID_airHoldPower,		//	paramOutputDownPower,	
																					cParameter::valID_airHoldDuration,	//	paramOutputDownDuration	
																					cParameter::valID_airLimit			//	paramOutputLimi
																				};
static const cParameter::valID_et cSmoke::heatParamterID[paramOutputEnd] PROGMEM{
																					cParameter::valID_heatStartDelay,	//	paramOutputDelay,	
																					cParameter::valID_heatStartPower,	//	paramOutputUpPower,	
																					cParameter::valID_heatStartDuration,//	paramOutputUpDuration,	
																					cParameter::valID_heatSmokePower,	//	paramOutputSmokePower,	
																					cParameter::valID_heatHoldPower,	//	paramOutputDownPower,	
																					cParameter::valID_heatHoldDuration,	//	paramOutputDownDuration	
																					cParameter::valID_heatLimit			//	paramOutputLimi
																				};



cSmoke::cSmoke(cParameter * pParameter)
{	// cSmoke
	smokeState.smokeState=smokeStateInit;
	smokeState.airState.state=outputStateOff;
	smokeState.airState.timer=0;
	smokeState.heatState.state=outputStateOff;
	smokeState.heatState.timer=0;
	smokeState.cycleState.counter=0;
	smokeState.cycleState.timer=0;
	smokeState.cycleState.downTime=0;
	smokeState.cycleState.upTime=0;
	smokeState.inputVoltageState.voltageTimer=0;
	smokeState.inputVoltageState.batteryLimitADCVal=(1<<ADCbits)+1;
	smokeState.inputVoltageState.batteryLowCount=0;
	smokeState.sysTime=0;
	smokeState.lastTime=0;
	
	this->pParameter=pParameter;
}	// cSmoke

void cSmoke::process(void)
{	// smoke Process
	smokeState.sysTime=millis();
	switch (smokeState.smokeState)
	{	// switch smokeState
		case smokeStateInit:
			pinMode(resetOutPin,INPUT_PULLUP);
			digitalWrite(resetOutPin,HIGH);
			
			pinMode(LED_BUILTIN,OUTPUT);
			digitalWrite(LED_BUILTIN,LOW);
			
			analogReference(INTERNAL);
			readInputVoltageADC();
			readInputVoltageADC();
			readInputVoltageADC();
			smokeState.inputVoltageState.inputVoltageADC.set(readInputVoltageADC());
			setTimer(smokeState.inputVoltageState.voltageTimer,timerValVoltage);

			PWM16.begin();
			PWM16.attach(airPin);
			PWM16.setPeriod(airPin,0);
			PWM16.attach(heatPin);
			PWM16.setPeriod(heatPin,0);
			
			smokeState.smokeState=smokeStateIdle;
			break;
		case smokeStateIdle:
			/* fallthrough */
		case smokeStateStart:
			/* fallthrough */
		case smokeStateActive:
			/* fallthrough */
		case smokeStatePause:
			processCycle();
			break;
		default:
			smokeState.smokeState=smokeStateInit;
			break;
	}	// switch smokeState
	
	// check input voltage
	checkInputVoltage();
	
	smokeState.lastTime=smokeState.sysTime;
}	// smoke Process

void cSmoke::enable(bool bEnable)
{	// enable
	if (bEnable)
	{	// enbable smoke if not allread active
		if (!isEnabled())
		{	// inactive -> enable
			smokeState.smokeState=smokeStateStart;
		}	// inactive -> enable
	}	// enbable smoke if not allread active
	else
	{
		if (isEnabled())
		{	// active -> disable
			smokeState.smokeState=smokeStateIdle;
		}
	}
}	// enable

bool cSmoke::isEnabled(void)
{ // isEnabled
	return smokeState.smokeState>smokeStateIdle;
} // isEnabled

void cSmoke::HWreset(void)
{	// HWreset
	// turn off heat and air
	PWM16.setPeriod(airPin,0);
	PWM16.setPeriod(heatPin,0);

	// activate Reset (D17 wired to RST)
	digitalWrite(resetOutPin,HIGH);
	pinMode(resetOutPin,OUTPUT);
	digitalWrite(resetOutPin,LOW);
	
	// done wait...
	while (true) __asm__("nop\n\t");
}	// HWreset

/* *********************************************************************
 * private functions
 * ********************************************************************* 
 */

void cSmoke::processCycle(void)
{	// processCycle
	switch (smokeState.smokeState)
	{	// switch smoke state
		case smokeStateStart:
			digitalWrite(LED_BUILTIN,HIGH);
			calcBatteryLowLimit();
			setCycleStart();
			setTimer(smokeState.cycleState.timer,smokeState.cycleState.upTime);
			smokeState.smokeState=smokeStateActive;
			break;
		case smokeStateActive:
			digitalWrite(LED_BUILTIN,HIGH);
			if (pParameter->getVal(cParameter::valID_periode)>0)
			{	// pulse timer is set, process
				if (checkTimer(smokeState.cycleState.timer,smokeState.cycleState.downTime))
				{	// timer has expired start off phase
					smokeState.smokeState=smokeStatePause;
				}	// timer has expired start off phase
			}	// pulse timer is set, process
			break;
		case smokeStatePause:
			digitalWrite(LED_BUILTIN,LOW);
			if (pParameter->getVal(cParameter::valID_periode)>0)
			{	// pause timer is set, process
				if (checkTimer(smokeState.cycleState.timer,smokeState.cycleState.upTime))
				{	// timer has expired check on phase
					if (smokeState.cycleState.counter)
						smokeState.cycleState.counter--;
					if ((smokeState.cycleState.counter>0) ||
						(pParameter->getVal(cParameter::valID_sequence)==0))
					{	// cycles left or continuous operation
						smokeState.smokeState=smokeStateActive;
					}	// cycles left or continuous operation
					else
					{	// no cycles left, turn off
						smokeState.smokeState=smokeStateIdle;
					}	// no cycles left, turn off
				}	// timer has expired ceck on phase
			}	// pause timer is set, process
			break;
		case smokeStateIdle:
			digitalWrite(LED_BUILTIN,LOW);
			break;
		default:
			break;
	}	// switch smoke state

	processOutput(smokeState.airState,airPin,airParamterID);
	processOutput(smokeState.heatState,heatPin,heatParamterID);
}	// processCycle


void cSmoke::processOutput(	outputState_st & outputState, 		// state variable
							const cPWM16::pwmPin_et outPin,		// output pin
							const cParameter::valID_et* idList)	// list with val ID
{	// processOutput
	switch (outputState.state)
	{	// switch outputState.state
		case outputStateOff:
			// do nothing
			if ((smokeState.smokeState>smokeStateIdle) &&
				(smokeState.smokeState!=smokeStatePause))
				outputState.state=outputStateStart;
			break;
		case outputStateStart:
			setTimer(	outputState.timer,
						getOutputParameter(idList,paramOutputDelay));
			outputState.state=outputStateDelay;
			break;
		case outputStateDelay:
			if (checkTimer(outputState.timer,-1))
			{	// timer expired, start up smoke
				setTimer(	outputState.timer,
							getOutputParameter(idList,paramOutputUpDuration));
				PWM.setPeriod(outPin,calcPowerSetting(idList,paramOutputUpPower));
				outputState.state=outputStateUp;
			}	// timer expired, start up smoke
			break;
		case outputStateUp:
			if (checkTimer(outputState.timer,-1))
			{	// timer expired,  smoke
				PWM.setPeriod(outPin,calcPowerSetting(idList,paramOutputSmokePower));
				outputState.state=outputStateSmoke;
			}	// timer expired,  smoke
			break;
		case outputStateSmoke:
			if (smokeState.smokeState!=smokeStateActive)
			{	// shut down
				setTimer(	outputState.timer,
							getOutputParameter(idList,paramOutputDownDuration));
				PWM.setPeriod(	outPin,
								calcPowerSetting(idList,paramOutputDownPower));
				outputState.state=outputStateDown;
			}	// shut down
			break;
		case outputStateDown:
			if (checkTimer(outputState.timer,-1))
			{	// timer expired, turn off smoke
				/*
				PWM.setPeriod(	outPin,
								calcPowerSetting(idList,paramOutputSmokePower));
				*/
				PWM.setPeriod(outPin,0);
				outputState.state=outputStateEnd;
			}	// timer expired, turn off smoke
			break;
		case outputStateEnd:
			// turn output off
			PWM16.setPeriod(outPin,0);	
			outputState.state=outputStateOff;
			break;
		default:
			break;
	}	// switch outputState.state
}	// processOutput


uint16_t cSmoke::getInputVoltage_mV(void)
{	// getInputVoltage_mV
	uint32_t voltage_mV;
	
	// get filtered adc readings
	// 10bit 1,1V, -> 1bit = 1,074.. mV
	voltage_mV=smokeState.inputVoltageState.inputVoltageADC.get();
	
	// first calculate voltage divider
	voltage_mV*=Rscale;
	// scale calculate reference
	voltage_mV*=referenceADCmV;	// internal reference in mV
	voltage_mV>>=ADCbits;		// reduce by adc resolution
	
	return voltage_mV;
}	// getInputVoltage_mV

bool cSmoke::isBatteryLow(void)
{	// isBatteryLow
	return 	(smokeState.smokeState>smokeStateStart) &&							// is allready running 
			(smokeState.inputVoltageState.batteryLowCount>batteryLimitCount);	// and battery low
}	// isBatteryLow


/* *********************************************************************
 * private functions
 * ********************************************************************* */

bool cSmoke::checkTimer(uint16_t &timer, uint16_t timeVal)
{	//	checkTimer
	uint16_t temp=smokeState.sysTime-smokeState.lastTime;
	bool bTimeout;

	if (timer>temp) 
		timer-=temp;
	else 
		timer=0;

	bTimeout=!timer;

	if (bTimeout)
	{
		setTimer(timer,timeVal);
	}
	return bTimeout;
}	//	checkTimer

void cSmoke::setTimer(uint16_t & timer, uint16_t timeVal)
{	//	setTimer
	timer=timeVal;
}	//	setTimer

void cSmoke::setCycleStart(void)
{	// setCycleStart
	uint32_t baseTime;
	
	// calc upTime
	baseTime=pParameter->getVal(cParameter::valID_periode);
	baseTime*=pParameter->getVal(cParameter::valID_dutyCycle);
	baseTime/=100;
	smokeState.cycleState.upTime=(uint16_t)baseTime;
	
	// calc downTime
	smokeState.cycleState.downTime=
		pParameter->getVal(cParameter::valID_periode) - 
		smokeState.cycleState.upTime;

	smokeState.cycleState.counter=pParameter->getVal(cParameter::valID_sequence);
	
	#ifdef TEST_5V_USB
		Serial.begin(115200);
		Serial.println();
		Serial.println("Cycle Parameter:");
		Serial.print("Duty Cycle (%): "); Serial.println(pParameter->getVal(cParameter::valID_dutyCycle));
		Serial.print("Periode (ms)  : "); Serial.println(pParameter->getVal(cParameter::valID_periode));
		Serial.print("Up Time (ms)  : "); Serial.println(smokeState.cycleState.upTime);
		Serial.print("Down Time (ms):"); Serial.println(smokeState.cycleState.downTime);
		Serial.println();
		Serial.println("Heat Parameter:");
		Serial.print("Limit (mV)         : "); Serial.println(getOutputParameter(heatParamterID,paramOutputLimit));
		Serial.print("Start Delay (ms)   : "); Serial.println(getOutputParameter(heatParamterID,paramOutputDelay));
		Serial.print("Start Power (%)    : "); Serial.println(getOutputParameter(heatParamterID,paramOutputUpPower));
		Serial.print("Start Duration (ms): "); Serial.println(getOutputParameter(heatParamterID,paramOutputUpDuration));
		Serial.print("Op. Power (%)      : "); Serial.println(getOutputParameter(heatParamterID,paramOutputSmokePower));
		Serial.print("Hold Power (%)     : "); Serial.println(getOutputParameter(heatParamterID,paramOutputDownPower));
		Serial.print("Hold Duration (ms) : "); Serial.println(getOutputParameter(heatParamterID,paramOutputDownDuration));
		Serial.print("Start PWM          : "); Serial.println(calcPowerSetting(heatParamterID,paramOutputUpPower));
		Serial.print("Operating PWM      : "); Serial.println(calcPowerSetting(heatParamterID,paramOutputSmokePower));
		Serial.print("Hold PWM           : "); Serial.println(calcPowerSetting(heatParamterID,paramOutputDownPower));
		Serial.println(); 
		Serial.println("Air Parameter:");
		Serial.print("Limit (mV)         : "); Serial.println(getOutputParameter(airParamterID,paramOutputLimit));
		Serial.print("Start Delay (ms)   : "); Serial.println(getOutputParameter(airParamterID,paramOutputDelay));
		Serial.print("Start Power (%)    : "); Serial.println(getOutputParameter(airParamterID,paramOutputUpPower));
		Serial.print("Start Duration (ms): "); Serial.println(getOutputParameter(airParamterID,paramOutputUpDuration));
		Serial.print("Op. Power (%)      : "); Serial.println(getOutputParameter(airParamterID,paramOutputSmokePower));
		Serial.print("Hold Power (%)     : "); Serial.println(getOutputParameter(airParamterID,paramOutputDownPower));
		Serial.print("Hold Duration (ms) : "); Serial.println(getOutputParameter(airParamterID,paramOutputDownDuration));
		Serial.print("Start PWM          : "); Serial.println(calcPowerSetting(airParamterID,paramOutputUpPower));
		Serial.print("Operating PWM      : "); Serial.println(calcPowerSetting(airParamterID,paramOutputSmokePower));
		Serial.print("Hold PWM           : "); Serial.println(calcPowerSetting(airParamterID,paramOutputDownPower));
		Serial.println();
		Serial.println("Battery Parameter:");
		Serial.print("Limit (mv)  : "); Serial.println(pParameter->getVal(cParameter::valID_batteryLimit));
		Serial.print("Voltage (mV): "); Serial.println(getInputVoltage_mV());
		Serial.end();
	#endif // TEST_5V_USB
}	// setCycleStart


uint16_t cSmoke::readInputVoltageADC(void)
{	
	#ifndef TEST_5V_USB
		return analogRead(inputVoltageADCPin);
	#else
		return 365; // debug =9V 
	#endif // TEST_5V_USB
}

uint16_t cSmoke::getOutputParameter(const cParameter::valID_et * idList, 
									paramOutput_et paramOutput) 
{	// getOutputParameter
	return pParameter->getVal(pgm_read_byte(&idList[paramOutput]));
}	// getOutputParameter

uint16_t cSmoke::calcPowerSetting(	const cParameter::valID_et * idList, 
									paramOutput_et paramOutput)
{	//	calcPowerSetting
	uint32_t power=0;
	uint16_t inputVolage_mV=getInputVoltage_mV();
	
	// calc power in mV
	power=getOutputParameter(idList,paramOutputLimit);
	power*=getOutputParameter(idList,paramOutput);
	power/=100;
	
	// scale to input voltage
	// scale for 10bit pwm is 1024*power/input_voltage
	if (power<inputVolage_mV)
	{	// pwm ok, calculate value
		power<<=10;
		power/=inputVolage_mV;
	}	// pwm ok, calculate value
	else
	{	// error
		// input voltage is way to low
		// disable PWM
		power=0;
		// ignore guard time an set battery low
		smokeState.inputVoltageState.batteryLowCount=batteryLimitCount+1;
	}	// error
	
	return (uint16_t)power;
}	//	calcPowerSetting

void cSmoke::calcBatteryLowLimit(void)
{	// calcBatteryLowLimit
	uint32_t limit;
	limit=pParameter->getVal(cParameter::valID_batteryLimit);
	limit<<=ADCbits;
	limit/=Rscale;
	limit/=referenceADCmV;
	smokeState.inputVoltageState.batteryLimitADCVal=limit;
	smokeState.inputVoltageState.batteryLowCount=0;
}	// calcBatteryLowLimit

void cSmoke::checkInputVoltage(void)
{
	if (checkTimer(smokeState.inputVoltageState.voltageTimer,timerValVoltage))
	{	// timer expired
		smokeState.inputVoltageState.inputVoltageADC.add(readInputVoltageADC());
		if (smokeState.inputVoltageState.batteryLowCount<=batteryLimitCount) 
		{	// not yet battery low
			if (smokeState.inputVoltageState.batteryLimitADCVal>
				smokeState.inputVoltageState.inputVoltageADC.get())
				smokeState.inputVoltageState.batteryLowCount++;
			else
				smokeState.inputVoltageState.batteryLowCount=0;
		}	// // not yet battery low
	}	// timer expired

}
