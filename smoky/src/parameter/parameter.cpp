#include "parameter.h"
#include <EEPROM.h>


static const uint16_t cParameter::defaultVal[valID_end] PROGMEM =
													{	  100,	// 	valID_dutyCycle, default duty cycle is 100% = continuous 
														    0,	//	valID_periode, default period is 0 ms = continous operation
														    0,	//	valID_sequence, default sequence count is 0 = continuous operation
														 3400,	//	valID_heatLimit, default 100% heating, conservative stting 3,4 (max 1S Lipo ~3,7V)
														    0,	//	valID_heatStartDelay, default 0 ms, no delay on preheat
														  100,	//	valID_heatStartPower, preheat at 100%
														    0,  //	valID_heatStartTime, 0 ms, no preheat
														  100,	//	valID_heatSmokePower, steam at 100%
														  100,	//	valID_heatHoldPower, hold power 100%
														    0,	//	valID_heatHoldTime, 0 ms no hold
														 5000,	//	valID_airLimit, default air limit for 5 V air pump
														    0,	//	valID_airStartDelay, no start delay or pre flow
														  100,	//	valID_airStartPower, start up ower 100%
														    0,	//	valID_airStartTime, 0 ms ?= no startup
														  100,	//	valID_airSmokePower, 100% air while s
														  100,	//	valID_airHoldPower,hold power 100%
														    0,	//	valID_airHoldTime, 0ms no holf
														 7000,	//	valID_batteryLow, typ. 2S LiPo 3.5V/cell
													};

static const uint16_t cParameter::absMaxVal[valID_end] PROGMEM=	//	absolute maximum values 
													{	  100,	// 	valID_dutyCycle (%)		
														60000,	//	valID_periode (ms)		
														   50,	//	valID_sequence (counts)	
														 4200,	//	valID_heatLimit (mV)
														  750,	//	valID_heatStartDelay (ms)
														  150,	//	valID_heatStartPower (%)
														  750,  //	valID_heatStartTime (ms)	
														  150,	//	valID_heatSmokePower (%)
														  150,	//	valID_heatHoldPower	(%)
														  750,	//	valID_heatHoldTime (ms)
														 7000,	//	valID_airLimit (mV)
														  750,	//	valID_airStartDelay	(ms)
														  150,	//	valID_airStartPower	(%)
														  750,	//	valID_airStartTime (ms)
														  150,	//	valID_airSmokePower (%)
														  150,	//	valID_airHoldPower (%)	
														  750,	//	valID_airHoldTime (ms)	
														20000,	//	valID_batteryLow (mV)
													};

static const uint16_t cParameter::absMinVal[valID_end] PROGMEM=	//	absolute minimum values 
													{	    1,	// 	valID_dutyCycle (%)		
														    0,	//	valID_periode (ms)		
														    0,	//	valID_sequence (counts)	
														 2500,	//	valID_heatLimit (mV)
														    0,	//	valID_heatStartDelay (ms)
														    0,	//	valID_heatStartPower (%)
														    0,  //	valID_heatStartTime (ms)	
														    0,	//	valID_heatSmokePower (%)
														    0,	//	valID_heatHoldPower	(%)
														    0,	//	valID_heatHoldTime (ms)
														 2500,	//	valID_airLimit (mV)
														    0,	//	valID_airStartDelay	(ms)
														    0,	//	valID_airStartPower	(%)
														    0,	//	valID_airStartTime (ms)
														    0,	//	valID_airSmokePower (%)
														    0,	//	valID_airHoldPower (%)	
														    0,	//	valID_airHoldTime (ms)	
														 7000,	//	valID_batteryLow (mV)
													};



cParameter::cParameter(void)
{	// configure
	uint8_t index;
	for (index=0;index<valID_end;index++)
	{	
		readVal[index]=pgm_read_word(&defaultVal[index]);
	}
	
	Address=0;
}	// configure
		
uint16_t cParameter::getVal(valID_et valID)
{	// get a value from memory
	if (valID<valID_end)
		return readVal[valID];
	else
		return 0;
}	// get a value from memory

void cParameter::setVal(valID_et valID, uint16_t val)
{	// write a new value to eeprom if necessary
	if (valID<valID_end)
	{	// valid ID
		if (readVal[valID]!=val)
		{	// new value, write to EEPROM
			readVal[valID]=val;
			if (EEPROM.length()-Address>3)
			{	// not jet at end of memory, write single item
				EEPROM.update(Address++,valID);
				EEPROM.update(Address++,val & 0xFF);
				EEPROM.update(Address++,val >> 8);
				EEPROM.update(Address,valID_invalid);
			}	// not jet at end of memory, write single item
			else
			{	// out of memory, copy to start
				write();	
			}	// out of memory, copy to start
		}	// new value, write to EEPROM
	}	// valid ID
}	// write a new value to eeprom if necessary


uint16_t cParameter::getMinVal(valID_et valID)
{
	return (int16_t)pgm_read_word(&absMinVal[valID]);
}

uint16_t cParameter::getMaxVal(valID_et valID)
{
	return (int16_t)pgm_read_word(&absMaxVal[valID]);
}



void cParameter::read(void)
{	
	valID_et id;
	Address=0;
	do
	{	// loop throug memory
		EEPROM.get(Address,id);
		if (id!=valID_invalid)
		{	// not end of list 
			if (id<valID_end)
			{	// valid ID
				Address++;
				EEPROM.get(Address,readVal[id]);
				Address+=2;
			}	// valid ID
			else
				write();	// invalid id, rewrite memory
		}	// not end of list
	}	// loop throug memory
	while ((Address<EEPROM.length()) && (id != valID_invalid));
}

void cParameter::reset(void)
{	// reset data
	uint8_t index;
	for (index=0;index<valID_end;index++)
	{	
		readVal[index]=pgm_read_word(&defaultVal[index]);
	}
	
	Address=0;
	EEPROM.update(Address,valID_invalid);
}	// reset data


void cParameter::write(void)
{
	uint8_t index;
	
	Address=0;
	
	for (index=0; index<valID_end; index++)
	{
		EEPROM.update(Address++,index);
		EEPROM.update(Address++,readVal[index] & 0xFF);
		EEPROM.update(Address++,readVal[index] >> 8);
	}
	EEPROM.update(Address,valID_invalid);
}


