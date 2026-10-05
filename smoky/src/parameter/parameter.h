#ifndef __cParameter_h__
#define __cParameter_h__
#include <stdint.h>
#include <Arduino.h>

/* ****************************************
 * uses on chip eeprom to store parameters
 * 
 * parameters are solely stored as int16_t in order
 * to use a simplified TLV structure using only a
 * tag (1Byte) and a value (2Byte)
 * 
 * at currently 16 values, 48+1 Bytes are needed
 * resulting in max 12 complete sets or 341 single values
 * when the end is reached, memory will be rewritten
 */

/* *************************************************************************************
 * Parameter limits
 * 
 * The operation mode can be either configured as on / off or pulsed operation with 
 * sequence count repetitions. The behaviour of heating and air can be configured
 * sepertely
 * 
 *  ^	Operation
 *  !   !<--------T--------->
 *  !   !<-D->!
 *  !   +-----+       +-----+
 *  !   !     !       !     !
 *  !   !     !       !     !
 * -+---+-----+-------+-----+------->
 *  !   
 * 
 * 				 ^	Heat
 * 				 !   
 * 			Ph1	 +-------+   
 * 				 !       !<--D-->      
 * 			Ph2	 +       +--...--+     
 * 				 !               !     
 * 		 	Ph3	 +               +-------+
 *				 !                       !
 * 				-+-------+---...-+-------+------->
 * 				  <-th1->         <-th2-> 
 * 		
 * 				 ^	Air
 * 		      ta0!   
 * 	Pa1	      +--+-------+   
 * 		      !  !       !<--D-->      
 * 	Pa2	      !  +       +--...--+     
 * 		      !  !               !     
 *  Pa3	      !  +               +-------+
 *		      !  !                       !
 * 		------+--+-------+---...-+-------+------->
 * 			   <--ta1--->         <-ta2-> 
 * 
 * if the Operation is continuous, cParameter::valID_dutyCycle "D" has to be set to 100% and
 * the cParameter::valID_sequence has to be set to 0 repetitions. If this parameter is !=0
 * the device will turn on for n times T. If the cParameter::valID_dutyCycle "D" is less 
 * than 100% the device will produce n puffs off smoke of duration D times T 
 * 
 * 
 * the heat can be configured in 3 cycles
 * preheat	: cParameter::valID_heatStartPower Ph1 and cParameter::valID_heatStartPower th1
 * operation: cParameter::valID_heatSmokePower Ph2
 * cooling	: cParameter::valID_heatHoldPower Ph3 and cParameter::valID_heatHoldDuration th2,
 *
 * the heat can be configured in 4 cycles
 * delay	: cParameter::valID_airStartDelay ta0 delay between heaat and air, positive or negative
 * startup	: cParameter::valID_airStartPower Pa1 and cParameter::valID_airStartDuration ta1
 * operation: cParameter::valID_airSmokePower Pa2
 * cooling	: cParameter::valID_airHoldPower Pa3 and cParameter::valID_airHoldDuration
 * 
 * Both air and heat sequences are started with the beginning of a power cycle.
 * when the start sequence is finished the system will stay within the smoke cycle.
 * when the powerer cycle transitions to an idle cycle the system will proceed with the hold cycle.
 * the hold cycle is either terminated by its timing constraints or the beginning of a new poer cycle
 *
 * special care should be taken to the power cycle. it shold be longer than the start cycle 
 * Air and heat power ar with respekt to the limit settings cParameter::valID_heatLimit 
 * and cParameter::valID_airLimit
 * 
 * therfore there are some limitations
 * cParameter::valID_dutyCycle:
 * limited to 1% ... 100%
 * 
 * cParameter::valID_periode
 * arbitrary limited to 5 s
 * 
 * cParameter::valID_sequence
 * arbitrary limited to 50
 * 
 * cParameter::valID_heatLimit
 * arbitrary limited to 2.5 V .. 4.2 V (1S max)
 * 
 * cParameter::valID_heatStartDelay
 * arbitrary limited to 0...750 ms

 * cParameter::valID_heatStartPower 
 * arbitrary limited to 0...150% of valID_heatLimit
 *
 * cParameter::valID_heatStartDuration
 * arbitrary limited to 0...750 ms 
 * 
 * cParameter::valID_heatSmokePower
 * arbitrary limited to 0...150% of valID_heatLimit
 * 
 * cParameter::valID_heatHoldPower
 * arbitrary limited to 0...150% of valID_heatLimit
 * 
 * cParameter::valID_heatHoldDuration
 * arbitrary limited to 0...750 ms
 * 
 * cParameter::valID_airLimit
 * arbitrary limited to 2.5 V .. 7 V (40% overload)
 * 
 * cParameter::valID_airStartDelay
 * arbitrary limited to 0...750 ms
 * 
 * cParameter::valID_airStartPower
 * arbitrary limited to 0...150% of valID_airLimit
 * 
 * cParameter::valID_airStartDuration
 * arbitrary limited to 0...750 ms
 * 
 * cParameter::valID_airSmokePower
 * arbitrary limited to 0...150% of valID_airLimit
 * 
 * cParameter::valID_airHoldPower
 * arbitrary limited to 0...150% of valID_airLimit
 * 
 * cParameter::valID_airHoldDuration
 * arbitrary limited to 0...750 ms
 * 
 * ****************************************************************************************************/

class cParameter
{
	public:
	
		typedef enum:uint8_t { 	valID_dutyCycle,
								valID_periode,
								valID_sequence,
								valID_heatLimit,
								valID_heatStartDelay,
								valID_heatStartPower,
								valID_heatStartDuration,
								valID_heatSmokePower,
								valID_heatHoldPower,
								valID_heatHoldDuration,
								valID_airLimit,
								valID_airStartDelay,
								valID_airStartPower,
								valID_airStartDuration,
								valID_airSmokePower,
								valID_airHoldPower,
								valID_airHoldDuration,
								valID_batteryLimit,
								valID_end,
								valID_invalid=255
							} valID_et;
		cParameter(void);
		
		uint16_t getVal(valID_et valID);
		void setVal(valID_et valID,uint16_t val);
		
		uint16_t getMinVal(valID_et valID);
		uint16_t getMaxVal(valID_et valID);
		
		
		void read(void);
		
		void reset(void);
	
	private:
		static const uint16_t defaultVal[valID_end] PROGMEM;	//	default values 
		static const uint16_t absMaxVal[valID_end] PROGMEM;	//	default values //
		static const uint16_t absMinVal[valID_end] PROGMEM;	//	default values 
		uint16_t readVal[valID_end];							// 	values read from eeprom

		uint16_t Address;									// current eeprom address
		
		void write(void);									// rewrite entire memory

};

#endif // __cParameter_h__
