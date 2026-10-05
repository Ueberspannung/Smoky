#include "menu.h"
#include "../ASCII/ASCII_ctrl.h"
#include "../tool/Version.h"

/*****************************************************
 * Font is (5+1 x 7+1)
 * 			Display		Performace (ms/char)
 * Size 1 26,7 x 10
 * Size 2 13,3 x 5		2,2
 * Size 3  8,9 x 3,3
 * Size 4  6,7 x 2,5
 *
 * Font				Display		Pixels	line  Offset Perf.
 * FreeMono9pt7b 	14,5 x 4,7	 11x16  19    11     1,7 ms/char
 */
#include <Fonts/FreeMono9pt7b.h>

#include "icondefs.h"
#include "menudefs.h"

static const uint16_t cMenu::decTab[] PROGMEM={ 1, 10, 100, 1000, 10000 };

											
cMenu::cMenu(void)
{
	menuState.menuItem=menuItemMain;
	menuState.menuPos=0;
	menuState.menuState=menuStateStart;
	menuState.currentTime=0;
	menuState.lastTime=0;
	menuState.buttonState.button=buttonOk;
	menuState.buttonState.held=false;
	menuState.buttonState.pressed=false;
	menuState.buttonState.released=false;

	charOffsetX=0;
	charOffsetY=0;
	charHeigth=0;
	charWidth=0;
}

void cMenu::process(void)
{
	Smoke.process();
	
	menuState.currentTime=millis();
	
	switch (menuState.menuState)
	{	// switch menuState
		case menuStateStart:
			display.begin(SSD1306_SWITCHCAPVCC, i2cAddress);
			 // text display tests
			display.clearDisplay();
			display.display();
			calcTextBounds();
			menuState.menuState=menuStateInfo;
			break;
		case menuStateInfo: 
			display.clearDisplay();
			printAt_P(0,0,Version_c().pgmGetVersion());
			printAt_P(0,1,Version_c().pgmGetBuiltDate());
			printIcon(iconsPerLine-1,0,iconSmoke,cursorOff);
			display.display();
			menuState.menuState=menuStateInfoPause;
			menuState.lastTime=menuState.currentTime;
			
			button_dn.init();
			button_lt.init();
			button_ok.init();
			button_rt.init();
			button_up.init();

			Parameter.read();
			break;
		case menuStateInfoPause:
			if ((uint16_t)(menuState.currentTime-menuState.lastTime)>infoDelay)
			{
				display.clearDisplay();
				display.display();
				menuState.menuItem=menuItemMain;
				menuState.menuPos=0;
				menuState.menuState=menuStateIdle;
				menuState.lastTime=menuState.currentTime;
				drawMenu();
			}
			break;
		case menuStateIdle:
			getInput();
			processMenu();
			//processItems();
			if (menuState.buttonState.pressed)  		// button pressed
			{
				drawMenu();
				menuState.lastTime=menuState.currentTime;
			}
			break;
		case menuStateActive:
			break;
		default:
			break;
	}	// switch menuState
}

/**********************************************************************
 * private functions
 *********************************************************************/
 
void cMenu::calcTextBounds(void)
{	// calcTextBounds
	uint8_t string[2];
	uint16_t x,y;
	int16_t min_x,min_y,max_x,max_y;
	
	display.setFont(&FreeMono9pt7b);
	display.setTextColor(WHITE, BLACK);
	display.setTextSize(1);

	x=16; min_x=x; max_x=x;
	y=16; min_y=y; max_y=y;
	 
	string[1]=0;
	for (string[0]=1; string[0]<128; string[0]++)
	{	// scan all chars
		int16_t  x1, y1;
		uint16_t w, h;

		display.getTextBounds((char*)string, x, y, &x1, &y1, &w, &h);	
				
		if (x1<min_x)		min_x=x1;
		if (y1<min_y)		min_y=y1;
		if ((int16_t)(x1+w)>max_x)	max_x=x1+w;
		if ((int16_t)(y1+h)>max_y)	max_y=y1+h;
	}	// scan all chars

	charOffsetX	=x-min_x;
	charOffsetY	=y-min_y;
	charWidth	=max_x-min_x;
	charHeigth	=max_y-min_y;
}	// calcTextBounds

void cMenu::printChar(uint8_t x, uint8_t y, uint8_t Char, cursor_et cursor)
{	// printChar
	uint8_t pos_x, pos_y;

	
	pos_y=(display_heigth>>1)*y+(((display_heigth>>1)-charHeigth)>>1);
	pos_x=charWidth*x;

	switch (cursor)
	{   // draw box or line
        case cursorFrame:
			display.drawRect(	pos_x, pos_y, charWidth, charHeigth, WHITE);
            break;
        case cursorTray:
			display.drawFastVLine(	pos_x, 
									pos_y + charHeigth - 1 - (charWidth>>1), 
									charWidth>>1, 
									WHITE);
			display.drawFastVLine(	pos_x + charWidth - 1, 
									pos_y + charHeigth - 1 - (charWidth>>1), 
									charWidth>>1,
									WHITE);
			display.drawFastHLine(	pos_x, 
									pos_y + charHeigth - 1, 
									charWidth, 
									WHITE);
			break;
        case cursorLine:
			display.drawFastHLine(	pos_x,
									pos_y + charHeigth -1, 
									charWidth, 
									WHITE);
			display.drawFastHLine(	pos_x,
									pos_y + charHeigth -2, 
									charWidth, 
									WHITE);
			break;
		case cursorBlock:
			display.fillRect(	pos_x, pos_y, charWidth, charHeigth, WHITE );
			break;
        default:
            break;
	}   // draw box or line
	display.drawChar(	pos_x + charOffsetX,
						pos_y + charOffsetY,
						Char,
						(cursor==cursorBlock)?(BLACK):(WHITE),
						(cursor==cursorBlock)?(WHITE):(BLACK),
						1);

}	// printChar

void cMenu::printAt(uint8_t x, uint8_t y, const char * text)
{
	printAt(x ,y , text, -1 ,-1, cursorOff);
}

void cMenu::printAt(uint8_t x, uint8_t y, const __FlashStringHelper * text)
{
	printAt_P(x ,y ,(const char *) text, -1 ,-1, cursorOff);
}
void cMenu::printAt_P(uint8_t x, uint8_t y, const char * text)
{
	printAt_P(x ,y , text, -1 ,-1, cursorOff);
}


void cMenu::printAt(uint8_t x, uint8_t y, const char * text , uint8_t cursorX, uint8_t cursorY,  cursor_et cursorMode)
{	// printAt (RAM)
	while (*text)
	{	// print chars from string
		printChar(x,y,*text,((x==cursorX) && (y==cursorY))?(cursorMode):(cursorOff));
		x++;
		text++;
	}	// print chars from string
}	// printAt (RAM)

void cMenu::printAt(uint8_t x, uint8_t y, const __FlashStringHelper * text , uint8_t cursorX, uint8_t cursorY,  cursor_et cursorMode)
{	// printAt (F())
	printAt_P(x, y, (const char*)text, cursorX,cursorY,cursorMode);
}	// printAt (F())

void cMenu::printAt_P(uint8_t x, uint8_t y, const char * text , uint8_t cursorX, uint8_t cursorY,  cursor_et cursorMode)
{	// printAt (PROGMEM)
	while (pgm_read_byte(text))
	{	// print chars from string
		printChar(x,y,pgm_read_byte(text),((x==cursorX) && (y==cursorY))?(cursorMode):(cursorOff));
		x++;
		text++;
	}	// print chars from string
}	// printAt (PROGMEM)

void cMenu::printIcon(uint8_t x, uint8_t y, icons_et icon, cursor_et cursorMode)
{	// printIcon
	uint8_t pos_x, pos_y;

	pos_x=0;
	if (x>0) pos_x=(display_width-iconsPerLine*iconWidth) % (iconsPerLine-1);
	pos_x+=((display_width-iconsPerLine*iconWidth) / (iconsPerLine-1) + iconWidth)*x;

	pos_y=(display_heigth/display_lines)*y;

	switch (cursorMode)
	{   // draw box or line
        case cursorFrame:
			display.drawRect(	pos_x, pos_y, iconWidth, iconHeigth, WHITE);
            break;
        case cursorTray:
			display.drawFastVLine(	pos_x, 
									pos_y + iconHeigth - 1 - (iconWidth>>1), 
									iconWidth>>1, 
									WHITE);
			display.drawFastVLine(	pos_x + iconWidth - 1, 
									pos_y + iconHeigth - 1 - (iconWidth>>1), 
									iconWidth>>1,
									WHITE);
			display.drawFastHLine(	pos_x, 
									pos_y + iconHeigth - 1, 
									iconWidth, 
									WHITE);
			break;
        case cursorLine:
			display.drawFastHLine(	pos_x,
									pos_y + iconHeigth -1, 
									iconWidth, 
									WHITE);
			display.drawFastHLine(	pos_x,
									pos_y + iconHeigth -2, 
									iconWidth, 
									WHITE);
			break;
		case cursorBlock:
			display.fillRect(	pos_x, pos_y, iconWidth, iconHeigth, WHITE );
			break;
        default:
            break;
	}   // draw box or line
	display.drawBitmap(	pos_x, pos_y, iconData[icon], iconWidth, iconHeigth,
						(cursorMode==cursorBlock)?(BLACK):(WHITE));

}	// printIcon


void cMenu::drawMenu(void)
{	// drawMenu
	display.clearDisplay();
	
	// draw upper line
	if (getMenuItem(menuState.menuItem,1)>menuItemEnd)
	{	// current selection is final function
		// display last menu in upper Line
		drawMenuLine(0,(menuItem_et)pgm_read_byte(&menuBack[menuState.menuItem]),menuState.menuPos,cursorOff);
	}	// current selection is final function
	else
	{	// current selection has menu
		drawMenuLine(0,menuState.menuItem,menuState.menuPos,cursorFrame);
	}	// current selection has menu


	// draw lower line
	switch(getMenuItem(menuState.menuItem,1))
	{	// select menu or special function
		case menuItemData:
			/* fallthrough */
		case menuItemEdit:		// dataItem selected
			drawMenuData(menuState.menuItem,menuState.menuData,menuState.menuPos,cursorFrame);
			break;
		case menuItemProcess:	// processItem slected show animation
			switch(menuState.menuItem)
			{	// special treatment
				case menuItemInfo:
					printAt_P(0,1,Version_c().pgmGetVersion());
					break;
				case menuItemSmoke:
					printIcon(menuState.menuPos,1,iconSmoke,cursorOff);
					printIcon(0,1,iconSmoke,cursorBlock);
					break;
				default:
					break;
			}	// special treatment
			break;
		case menuItemConfirm:
			drawMenuLineConfirm(1,menuState.menuItem,menuState.menuPos,cursorFrame);
			break;
		default:	// regular menu item
			if (menuState.menuPos>0)						// check if next menu is data or menu
			{	// lower line needs to be drawn
				switch (getMenuItem(getMenuItem(menuState.menuItem,menuState.menuPos),1))
				{	// Menu / data
					case menuItemEdit:
						/* fallthrough */
					case menuItemData:
						drawMenuData(	getMenuItem(menuState.menuItem,menuState.menuPos),
										menuState.menuData,
										menuState.menuPos,
										cursorOff);
						break;
					case menuItemProcess:
						switch(getMenuItem(menuState.menuItem,menuState.menuPos))
						{	// special treatment
							case menuItemInfo:
								printAt_P(0,1,Version_c().pgmGetVersion());
								break;
							case menuItemSmoke:
								printIcon(0,1,iconSmoke,cursorOff);
								// to do move smoke !
								break;
							default:
								break;
						}	// special treatment
						break;
					case menuItemConfirm:
						drawMenuLineConfirm(1,getMenuItem(menuState.menuItem,menuState.menuPos),-1,cursorOff);
						break;
					default:	// regular mennu line
						drawMenuLine(1,getMenuItem(menuState.menuItem,menuState.menuPos),menuState.menuPos,cursorOff);
						break;
				}	// Menu / data
			}	// lower line needs to be drawn
			break;
	}	// select menu or special function

	display.display();
}	// drawMenu

void cMenu::drawMenuLine(uint8_t line, menuItem_et menuItem, uint8_t cursorPos, cursor_et cursorMode)
{	// drawMenuLine
	uint8_t iconPos=0;

	while (getMenuItem(menuItem,iconPos)!=menuItemEnd)
	{ 	// go through menu items
		printIcon(	iconPos, line,
					getIcon(getMenuItem(menuItem,iconPos)),
					(iconPos==cursorPos)?(cursorMode):(cursorOff));
		iconPos++;
	}	// go through menu items
}	// drawMenuLine

void cMenu::drawMenuLineConfirm(uint8_t line, menuItem_et menuItem, uint8_t cursorPos, cursor_et cursorMode)
{	// drawMenuLine
	printIcon(	0, line,
				getIcon(menuItem),
				(cursorPos==0)?(cursorMode):(cursorOff));
	printIcon(	1, line,
				iconOk,
				(cursorPos==1)?(cursorMode):(cursorOff));
}	// drawMenuLine


void cMenu::drawMenuData(menuItem_et menuItem, uint16_t data, uint8_t cursorPos, cursor_et cursorMode)
{	// drawMenuData
	uint8_t digit;
	uint8_t offset;
	printIcon(	0, 1,
				getIcon(menuItem),
				(cursorPos==0)?(cursorMode):(cursorOff));
	printIcon(	iconsPerLine-1,1,
				iconCancel,
				(cursorPos==getMenuValDigits(menuItem)+1)?(cursorMode):(cursorOff));
	/*
	if (data>0)
	{	// positive, print space
		printChar(1,dataOffset,SPACE,cursorOff);
	}	// positive, print space
	else
	{	// negative, print "-"
		printChar(1,dataOffset,'-',cursorOff);
		data=-data;
	}	// negative, print "-"
	*/
	
	digit=getMenuValDigits(menuItem);
	offset=(display_width-(digit+3)*charWidth)/2/charWidth;
	while (digit>0)
	{	// write digits
		printChar(offset+digit,1,(data%10)+'0',(cursorPos==digit)?(cursorMode):(cursorOff));
		data/=10;
		digit--;
	}	// write digits
	
	switch(getMenuValUnit(menuItem))
	{	// print unit
		case unit_mV:
			printAt(offset+getMenuValDigits(menuItem)+1,1,F(" mV"));
			break;
		case unit_ms:
			printAt(offset+getMenuValDigits(menuItem)+1,1,F(" ms"));
			break;
		case unit_percent:
			printAt(offset+getMenuValDigits(menuItem)+1,1,F(" %"));
			break;
		default:
			break;
	}	// print unit
}	// drawMenuData

uint8_t cMenu::getMenuIndex(menuItem_et menuItem)
{	// getMenuIndex
	uint8_t index=0;
	while (	(pgm_read_byte(&menuList[index])!=(uint8_t)menuItemEnd) &&
			(pgm_read_byte(&menuList[index])!=(uint8_t)menuItem) )
	{	// end of list not reached
		
		if ((uint8_t)menuItem!=pgm_read_byte(&menuList[index]))
		{	// no match, next menu
			while (pgm_read_byte(&menuList[index++])!=(uint8_t)menuItemEnd);
		}	// no match, next menu
	}	// end of list not reached
	
	return index;
}	// getMenuIndex


uint8_t cMenu::getMenuOffset(menuItem_et menu, menuItem_et item)
{	// getMenuOffset
	uint8_t menuIndex=getMenuIndex(menu);
	uint8_t offset=0;
	
	if (pgm_read_byte(&menuList[menuIndex])==menu)
	{	// found menu, search item
		while (	(pgm_read_byte(&menuList[menuIndex+offset])!=item) &&
				(pgm_read_byte(&menuList[menuIndex+offset])!=menuItemEnd) )
				offset++;
		if (pgm_read_byte(&menuList[menuIndex+offset])!=item) offset=0;
	}	// found menu, search item
	return offset;
}	// getMenuOffset


cMenu::menuItem_et cMenu::getMenuItem(menuItem_et menuItem, uint8_t menuIndex)
{	// getMenuItem
	return (menuItem_et) pgm_read_byte(&menuList[getMenuIndex(menuItem)+menuIndex]);
}	// getMenuItem

cMenu::menuItem_et	cMenu::getPreviosMenu(menuItem_et menuItem)
{	// getPreviousMenu
	return (menuItem_et) pgm_read_byte(&menuBack[menuItem]);
}	// getPreviousMenu



uint8_t cMenu::getMenuValIndex(menuItem_et menuItem)
{	// getMenuValIndex
	uint8_t index=0;
	
	while (	(pgm_read_byte(&menuVals[index].menuItem)!=(uint8_t)menuItemEnd) && 
			(pgm_read_byte(&menuVals[index].menuItem)!=(uint8_t)menuItem) )
			index++;
	return index;
}	// getMenuValIndex

cParameter::valID_et cMenu::getMenuValID(menuItem_et menuItem)
{	// getMenuValID
	return pgm_read_byte(&menuVals[getMenuValIndex(menuItem)].valID);
}	// getMenuValID
	
uint8_t cMenu::getMenuValDigits(menuItem_et menuItem)
{	// getMenuValDigits
	return pgm_read_byte(&menuVals[getMenuValIndex(menuItem)].digits);
}	// getMenuValDigits

cMenu::unit_et	cMenu::getMenuValUnit(menuItem_et menuItem)
{	// getMenuValUnit
	return pgm_read_byte(&menuVals[getMenuValIndex(menuItem)].unit);
}	// getMenuValUnit

cMenu::icons_et cMenu::getIcon(menuItem_et menuItem)
{	// getIcon
	return pgm_read_byte(&menuIcons[menuItem]);
}	// getIcon

uint16_t cMenu::getDecimalPower(uint8_t pow)
{	// getDecimalPower
	return pgm_read_word(&decTab[pow]);
}	// getDecimalPower


void cMenu::getInput(void)
{	// getInput
	menuState.buttonState.button=
		menuState.buttonState.next;
	switch (menuState.buttonState.button)
	{	// switch button
		case buttonUp:
			menuState.buttonState.pressed	=button_up.pressed();
			menuState.buttonState.held		=button_up.long_press();
			menuState.buttonState.released	=button_up.released();
			break;
		case buttonDown:
			menuState.buttonState.pressed	=button_dn.pressed();
			menuState.buttonState.held		=button_dn.long_press();
			menuState.buttonState.released	=button_dn.released();
			break;
		case buttonLeft:
			menuState.buttonState.pressed	=button_lt.pressed();
			menuState.buttonState.held		=button_lt.long_press();
			menuState.buttonState.released	=button_lt.released();
			break;
		case buttonRight:
			menuState.buttonState.pressed	=button_rt.pressed();
			menuState.buttonState.held		=button_rt.long_press();
			menuState.buttonState.released	=button_rt.released();
			break;
		case buttonOk:
			menuState.buttonState.pressed	=button_ok.pressed();
			menuState.buttonState.held		=button_ok.long_press();
			menuState.buttonState.released	=button_ok.released();
			break;
		default:
			break;
	}	// switch button
	
	if (!menuState.buttonState.held)
	{	// button inactive -> next
		menuState.buttonState.next=(button_et)(menuState.buttonState.next+1);
		if (menuState.buttonState.next==buttonEnd)
			menuState.buttonState.next=buttonUp;
	}	// button inactive -> next
}	// getInput

void cMenu::processMenu(void)
{	// processInput
	switch (getMenuItem(menuState.menuItem,1))
	{	// switch menuState.menuItem,1
		case menuItemData:
			// should not be possible
			// you can't enter a data Menu
			// exept battery low
			processItemData();
			break;
		case menuItemEdit:
			processItemEdit();
			break;
		case menuItemProcess:
			processItemProcess();
			break;
		case menuItemConfirm:
			processItemConfirm();
			break;
		default:
			processItem();
			break;
	}	// menuState.menuItem,1

}	// processInput



void cMenu::processMenuNavigation(void)
{	// processMenuNavigation
	if (menuState.buttonState.pressed)
	{	// select action
		switch (menuState.buttonState.button)
		{	// switch button
			case buttonUp:
				previousMenuLevel();
				break;
			case buttonOk:
				/* fallthrough */
			case buttonDown:
				nextMenuLevel();
				break;
			case buttonLeft:
				previousMenuItem();
				break;
			case buttonRight:
				nextMenuItem();
				break;
			default:
				break;
		}	// switch button
	}	// select action
}	// processMenuNavigation

void cMenu::previousMenuItem(void)
{	// previous Menu Item
	if (menuState.menuPos>0) menuState.menuPos--;
	/*
	if (getMenuItem(menuState.menuItem,menuState.menuPos)==menuItemBatteryInfo)
		menuState.menuData=Smoke.getInputVoltage_mV();
	*/
}	// previous Menu Item

void cMenu::nextMenuItem(void)
{	// nextMenuItem
	if (getMenuItem(menuState.menuItem,menuState.menuPos+1)!=menuItemEnd)
		menuState.menuPos++;
	/*
	if (getMenuItem(menuState.menuItem,menuState.menuPos)==menuItemBatteryInfo)
		menuState.menuData=Smoke.getInputVoltage_mV();
	*/
}	// nextMenuItem


void cMenu::previousMenuLevel(void)
{	// previous Menu
	menuState.menuPos =	getMenuOffset(	getPreviosMenu(menuState.menuItem),
										menuState.menuItem);
	menuState.menuItem=	getPreviosMenu(	menuState.menuItem);
}	// previous Menu


void cMenu::nextMenuLevel(void)
{	// nextMenu			
	if (getMenuItem(getMenuItem(menuState.menuItem,menuState.menuPos),1)!=menuItemData)
	{	// check if submenu of next Menu is not Data
		menuState.menuItem=	getMenuItem(menuState.menuItem,menuState.menuPos);
		menuState.menuPos=0;
	}	// check if submenu of next Menu is not Data
}	// nextMenu



void cMenu::processItemEdit(void)
{	// processItemEdit
	switch (menuState.buttonState.button)
	{	// switch mode
		case buttonUp:
			if ( (menuState.buttonState.pressed) || 										// pressed
				 (menuState.buttonState.held &&												// or auto repeat
					((uint16_t)(menuState.currentTime-menuState.lastTime)>repeatTime)) )
			{ 	// button pressed or held
				if (menuState.menuPos==0)
				{	// first pos, save & exit
					if (menuState.buttonState.pressed)
					{	// only pressed vaild
						Parameter.setVal(getMenuValID(menuState.menuItem),menuState.menuData);
						previousMenuLevel();
					}	// only pressed vaild
				}	// first pos, save & exit
				else if (menuState.menuPos>getMenuValDigits(menuState.menuItem))
				{	// last pos discard an exit
					if (menuState.buttonState.pressed)
					{	// only pressed vaild
						menuState.menuData=Parameter.getVal(getMenuValID(menuState.menuItem));
						previousMenuLevel();
					}	// only pressed vaild
				}	// last pos discard an exit
				else
				{	// edit, increment  position
					if (Parameter.getMaxVal(getMenuValID(menuState.menuItem))-menuState.menuData>
						getDecimalPower(getMenuValDigits(menuState.menuItem)-menuState.menuPos))
						menuState.menuData+=
							getDecimalPower(getMenuValDigits(menuState.menuItem)-menuState.menuPos);
					else
						menuState.menuData=Parameter.getMaxVal(getMenuValID(menuState.menuItem));
					menuState.buttonState.pressed=true; // force update
				}	// edit, increment  position
			}	// button pressed or held
			break;
		case buttonDown:
			if ( (menuState.buttonState.pressed) || 										// pressed
				 (menuState.buttonState.held &&												// or auto repeat
					((uint16_t)(menuState.currentTime-menuState.lastTime)>repeatTime)) )
			{  // button pressed or held
				if (menuState.menuPos==0)
				{	// first pos, do nothing
					if (menuState.buttonState.pressed)
					{
						// only up possible
					}
				}	// first pos, do nothing
				else if (menuState.menuPos>getMenuValDigits(menuState.menuItem))
				{	// last pos do nothing
					if (menuState.buttonState.pressed)
					{
						// only up possible
					}
				}	// last pos do nothing
				else 
				{	// edit, decrement  position
					if (menuState.menuData > 
							Parameter.getMinVal(getMenuValID(menuState.menuItem)) + 
							getDecimalPower(getMenuValDigits(menuState.menuItem)-menuState.menuPos))
						menuState.menuData=
							menuState.menuData-
							getDecimalPower(getMenuValDigits(menuState.menuItem)-menuState.menuPos);
					else
						menuState.menuData=Parameter.getMinVal(getMenuValID(menuState.menuItem));
					menuState.buttonState.pressed=true; // force update
				}	// edit, decrement  position
			} // button pressed or held
			break;
		case buttonRight:
			if (menuState.buttonState.pressed)
			{ 	// button pressed
				if (menuState.menuPos<getMenuValDigits(menuState.menuItem)+1)
					menuState.menuPos++;
			}
			break;
		case buttonLeft:
			if (menuState.buttonState.pressed)
			{ 	// button pressed
				if (menuState.menuPos>0)
					menuState.menuPos--;
			}
			break;
		case buttonOk:
			if (menuState.buttonState.pressed)
			{ 	// button pressed
				if (menuState.menuPos==0)
				{	// first pos, save & exit
					Parameter.setVal(getMenuValID(menuState.menuItem),menuState.menuData);
					previousMenuLevel();
				}	// first pos, save & exit
				else if (menuState.menuPos>getMenuValDigits(menuState.menuItem))
				{	// last pos discard an exit
					menuState.menuData=Parameter.getVal(getMenuValID(menuState.menuItem));
					previousMenuLevel();
				}	// last pos discard an exit
			}	// edit, decrement  position
			break;
		default:
			break;
	}	// switch mode
}	// processItemEdit

void cMenu::processItemData(void)
{	// processItemConfirm
	// data Items can not be entered, 
	// so its the menu at the cursor position
	switch (getMenuItem(menuState.menuItem,menuState.menuPos))
	{	// switch regular data Item (next level)
		case menuItemBatteryInfo:
			processItemBatteryInfo();
			break;
		default:
			break;
	}	// switch regular data Item (next level)
	switch (menuState.menuItem)
	{	// switch emergency data item, (same level)
		case menuItemBatteryLow:
			processItemBatteryInfo();
			break;
		default:
			break;
	}	// switch emergency data item, (same level)

}	// processItemConfirm

void cMenu::processItemProcess(void)
{	// processItemProcess
	// process items wich have attribute menuItemProcess
	// currently only smoke and info
	switch (menuState.menuItem)
	{
		case menuItemInfo:
			processItemInfo();
			break;
		case menuItemSmoke:
			processItemSmoke();
			break;
		default:
			break;
	}
}	// processItemProcess

void cMenu::processItemConfirm(void)
{	// processItemConfirm
	if (menuState.buttonState.pressed)
	{	// menu action
		switch (menuState.buttonState.button)
		{	// switch button
			case buttonUp:
				/* fallthrough */
			case buttonLeft:
				/* fallthrough */
			case buttonRight:
				processMenuNavigation();
				break;
			case buttonDown:
				/* fallthrough */
			case buttonOk:
				if (getMenuItem(menuState.menuItem,menuState.menuPos)==menuItemConfirm)
				{	// action confirmed, process
					switch (menuState.menuItem)
					{	// select Item
						case menuItemReset:
							processItemReset();
							break;
						default: 
							break;
					}	// select Item
				}	// action confirmed, process
				break;
			default:
				break;
		}	// switch button
	}	// menu action
}	// processItemConfirm

void cMenu::processItem(void)
{	// process any item <itemEnd
	
	
	if (menuState.buttonState.pressed)
	{	//	no special treatment, switch menu level
		processMenuNavigation();
	}	//	no special treatment, switch menu level

	// if next Menu needs processing
	switch (getMenuItem(getMenuItem(menuState.menuItem,menuState.menuPos),1))
	{	
		case menuItemData:
			processItemData();
			break;
		case menuItemEdit:
			if (menuState.buttonState.pressed)
			{	// just selected menu with edit option
				menuState.menuData=
					Parameter.getVal(getMenuValID(getMenuItem(	menuState.menuItem,
																menuState.menuPos)));
			}	// just selected menu with edit option
			break;
		default:
			break;
	} 

	
}	// process any item <itemEnd

void cMenu::processItemBatteryInfo(void)
{
	if (menuState.buttonState.pressed || 
		((uint16_t)(menuState.currentTime-menuState.lastTime>updateTime)))
	{ // update Data
		menuState.menuData=Smoke.getInputVoltage_mV();
		menuState.buttonState.pressed=true;	// force redraw
	} // update Data
}


void cMenu::processItemInfo(void)
{	// processItemInfo
	// when entered display start message
	menuState.menuState=menuStateInfo;
}	// processItemInfo

void cMenu::processItemReset(void)
{	// processItemReset
	Smoke.HWreset();
}	// processItemReset

void cMenu::processItemSmoke(void)
{	// processItemSmoke
	if (menuState.buttonState.held ||
		menuState.buttonState.pressed ||
		menuState.buttonState.released)
	{	// menu action
		switch (menuState.buttonState.button)
		{
			case buttonUp:
				if (menuState.buttonState.pressed)
				{	// pressed
					if (Smoke.isEnabled())
						Smoke.enable(false);
					else
					{	// processMenuNavigation & exit
						processMenuNavigation();
						return;
					}	// processMenuNavigation & exit
				}	// pressed
				break;
			case buttonDown:
				/* fallthrough */
			case buttonOk:
				if (menuState.buttonState.pressed)
					Smoke.enable(!Smoke.isEnabled());
				else
					Smoke.enable(menuState.buttonState.held);
				break;
			default:
				break;
		}
	}	// menu action
	
	if (menuState.buttonState.pressed || 
		menuState.buttonState.released ||
		((uint16_t)(menuState.currentTime-menuState.lastTime)>updateTime))
	{	// update Smoke icon
		if (Smoke.isEnabled())
		{	// ative move icon
			menuState.menuPos++;
			if (menuState.menuPos==iconsPerLine)
				menuState.menuPos=1;
		}	// ative move icon
		else
			menuState.menuPos=0;
		if (Smoke.isBatteryLow())
		{	// battery low, lock system
			Smoke.enable(false);
			menuState.menuItem=menuItemBatteryLow;
			menuState.menuPos=-1;
		}	// battery low, lock system
		menuState.buttonState.pressed=true;	// force update
	}	// update Smoke icon
}	// processItemSmoke


