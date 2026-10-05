#ifndef __menu_h__
#define __menu_h__

#include "../parameter/parameter.h" 
#include "../button/button.h"
#include "../smoke/smoke.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


class cMenu
{
	public:
		cMenu(void);

		void process(void);
		
	private:
		typedef enum:uint8_t {	menuStateStart, 
								menuStateInfo, 
								menuStateInfoPause,
								menuStateIdle, 
								menuStateActive } menuState_et;
								
		typedef enum:uint8_t { editUp, editDn, editOk } edit_et;
		typedef enum:uint8_t {	// Leve 0
								menuItemMain, 		
								// Level 1 - menuItemMain
								menuItemSmoke,		
								menuItemSettings,
								menuItemInfo,
								// level 2 - menuItemSettings
								menuItemMode,		
								menuItemHeat,
								menuItemAir,
								menuItemBattery,
								menuItemReset,		
								// level 3 - menuItemMode
								menuItemDutyCycle,	
								menuItemPeriod,
								menuItemSequence,
								// level 3 - menuItemHeat
								menuItemHeatLimit,
								menuItemHeatStart,
								menuItemHeatSmokePower,
								menuItemHeatHold,
								// level 3 - menuItemAir
								menuItemAirLimit,
								menuItemAirStart,
								menuItemAirSmokePower,
								menuItemAirHold,
								// level 3 - menuItemBattery
								menuItemBatteryLimit,
								menuItemBatteryInfo,
								// level 4 - menuItemHeatStart
								menuItemHeatStartDelay,
								menuItemHeatStartPower,
								menuItemHeatStartDuration,
								// level 4 - menuItemHeatHold
								menuItemHeatHoldPower, 
								menuItemHeatHoldDuration,
								// level 4 - menuItemAirStart
								menuItemAirStartDelay,
								menuItemAirStartPower,	
								menuItemAirStartDuration,
								// level 4 - menuItemAirHold
								menuItemAirHoldPower,
								menuItemAirHoldDuration,
								// no selectable menu 
								menuItemBatteryLow,
								// MenuEnd
								menuItemEnd,
								menuItemEdit,
								menuItemData,
								menuItemProcess,
								menuItemConfirm
							} menuItem_et;
		
		typedef enum:uint8_t {	iconMenu,		// menu icon, hamburger
								iconSmoke,		// smoke cloud
								iconSettings,	// three sliders
								iconInfo,		// "i" in circle
								iconMode,		// two circular arrows
								iconHeat,		// heat icon form hot surface
								iconAir,		// wind icon
								iconDutyCycle,	// square wave with marked pulse and "%"
								iconPeriod,		// square wave with marked cycle and "T"
								iconSequence,	// three pulses and "n"
								iconHeatStart,	// iconHeat combined with arrow facing up
								iconHeatSmoke,	// iconHeat combined with icon iconSmoke
								iconHeatHold,	// iconHeat combined with arrow facing down
								iconAirStart,	// iconAir combined with arrow facing up
								iconAirSmoke,	// iconAir combined with icon iconSmoke
								iconAirHold,	// iconAir combined with arrow facing down
								iconLimitHigh,	// scale with arrow facing up to an dash
								iconLimitLow,	// scale with arrow facing up to an dash
								iconPower,		// dashed circle with flash
								iconDuration,	// segmented cricle  arrow
								iconDelay,		// coordinate grid with delayd pulse and "t"
								iconBattery,	// battery icon
								iconBatteryLow,	// strike out battery with exclamation mark
								iconBack,		// backwards curve arrow
								iconReset,		// circular arrow with dot at beginning / end
								iconCancel,		// x in circle
								iconOk,			// v in circle
								iconListEnd		// end of icon List
							}	icons_et;
		
		typedef enum:uint8_t { unit_percent, unit_ms, unit_mV, unit_none} unit_et;
		
		typedef enum:uint8_t { cursorOff, cursorLine, cursorTray, cursorFrame, cursorBlock } cursor_et;
		
		typedef enum:uint8_t { buttonUp, buttonDown, buttonLeft, buttonRight, buttonOk, buttonEnd } button_et;
		
		typedef struct val_s {	menuItem_et				menuItem;
								cParameter::valID_et	valID;
								uint8_t					digits;
								unit_et					unit;
							} val_st;
	
		typedef struct button_s { 	button_et	button;
									button_et	next;
									uint8_t		pressed:1;
									uint8_t		held:1;
									uint8_t		released:1;
								} button_st;
	
		typedef struct menuState_s {	menuState_et 	menuState;
										menuItem_et		menuItem;
										button_st		buttonState;
										uint8_t			menuPos;
										uint16_t		lastTime;
										uint16_t		currentTime;
										uint16_t			menuData;
									}	menuState_st;
		
		static const uint8_t display_addr	=0x3c;
		static const uint8_t display_width	=128;
		static const uint8_t display_heigth	=32;
		static const uint8_t display_reset	=-1;
		static const uint8_t display_lines	=2;
		static const uint8_t iconHeigth		=16;
		static const uint8_t iconWidth		=16;
		static const uint8_t iconSize		=32;
		static const uint8_t iconsPerLine	=6;
		static const uint8_t i2cAddress		=0x3c;
		
		static const uint8_t dataOffset		=2;
		static const uint16_t infoDelay		=5000;
		static const uint16_t updateTime	=500;
		static const uint16_t repeatTime	=100;
		
		static const uint8_t iconData[iconListEnd][iconSize] PROGMEM;
		
		static const icons_et menuIcons[menuItemEnd] PROGMEM;

		static const menuItem_et menuList[] PROGMEM;
		static const menuItem_et menuBack[menuItemEnd] PROGMEM;

		static const val_st menuVals[] PROGMEM;

		static const uint16_t decTab[] PROGMEM;

		Adafruit_SSD1306 display{display_width, display_heigth, &Wire,-1};
	
		button_c button_up{2};
		button_c button_dn{3};
		button_c button_lt{4};
		button_c button_rt{5};
		button_c button_ok{6};
	
	
		cParameter Parameter;

		cSmoke Smoke{&Parameter};
	
		menuState_st	menuState;

		uint8_t	charOffsetX;
		uint8_t	charOffsetY;
		uint8_t	charHeigth;
		uint8_t	charWidth;

		void calcTextBounds(void);
		void printChar(uint8_t x, uint8_t y, uint8_t Char, cursor_et cursor);
		void printAt(uint8_t x, uint8_t y, const char * text);
		void printAt(uint8_t x, uint8_t y, const __FlashStringHelper * text);
		void printAt_P(uint8_t x, uint8_t y, const char * text);
		void printAt(uint8_t x, uint8_t y, const char * text , uint8_t cursorX, uint8_t cursorY,  cursor_et cursorMode);
		void printAt(uint8_t x, uint8_t y, const __FlashStringHelper * text , uint8_t cursorX, uint8_t cursorY,  cursor_et cursorMode);
		void printAt_P(uint8_t x, uint8_t y, const char * text , uint8_t cursorX, uint8_t cursorY,  cursor_et cursorMode);
		
		
		void printIcon(uint8_t x, uint8_t y, icons_et icon, cursor_et cursorMode);
		
		void drawMenu(void);
		void drawMenuLine(uint8_t line, menuItem_et menuItem, uint8_t cursorPos, cursor_et cursorMode);
		void drawMenuLineConfirm(uint8_t line, menuItem_et menuItem, uint8_t cursorPos, cursor_et cursorMode);
		void drawMenuData(menuItem_et menuItem, uint16_t data, uint8_t cursorPos, cursor_et cursorMode);
		
		uint8_t getMenuIndex(menuItem_et menuItem);
		uint8_t getMenuOffset(menuItem_et menu, menuItem_et item=menuItemEnd);
		menuItem_et getMenuItem(menuItem_et menuItem, uint8_t menuIndex);
		menuItem_et	getPreviosMenu(menuItem_et menuItem);
		
		uint8_t getMenuValIndex(menuItem_et menuItem);
		cParameter::valID_et getMenuValID(menuItem_et menuItem);
		uint8_t getMenuValDigits(menuItem_et menuItem);
		unit_et	getMenuValUnit(menuItem_et menuItem);

		icons_et getIcon(menuItem_et menuItem);
		
		uint16_t getDecimalPower(uint8_t pow);
		
		void getInput(void);
		void processMenu(void);
		void processItemData(void);
		void processItemEdit(void);
		void processItemProcess(void);
		void processItemConfirm(void);
		void processItem(void);
		void processItemBatteryInfo(void);
		void processItemInfo(void);
		void processItemReset(void);
		void processItemSmoke(void);

		void processMenuNavigation(void);
		void previousMenuItem(void);
		void nextMenuItem(void);
		void previousMenuLevel(void);
		void nextMenuLevel(void);
};

#endif //__menu_h__
