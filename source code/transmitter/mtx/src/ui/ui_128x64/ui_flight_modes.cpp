#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_flight_modes()
{
  switch(theScreen)
  {
    case SCREEN_FLIGHT_MODES:
      {
        flight_mode_t *fmd = &Model.FlightMode[thisFmdIdx];
        
        if(isEditTextDialog)
        {
          editTextDialog(PSTR("Flght mode name"), fmd->name, sizeof(fmd->name), true, true, false);
          break;
        }
        
        drawHeader(extrasMenu[EXTRAS_MENU_FLIGHT_MODES]);
        
        //-- draw --
        display.setCursor(8, 9);
        
        getControlSwitchName(textBuff, CTRL_SW_FMD_FIRST + thisFmdIdx, sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        display.setCursor(0, 20);
        display.print(F("Name:"));
        display.setCursor(66, 20);
        if(isEmptyStr(fmd->name, sizeof(fmd->name)))
          display.print(F("--"));
        else
          display.print(fmd->name);
        
        display.setCursor(0, 29);
        display.print(F("Switch:"));
        display.setCursor(66, 29);
        if(thisFmdIdx == 0)
          display.print(F("N/A")); 
        else
        {
          getControlSwitchName(textBuff, fmd->swtch, sizeof(textBuff));
          display.print(textBuff);
        }
        
        display.setCursor(0, 38);
        display.print(F("Fade-in:"));
        display.setCursor(66, 38);
        printSeconds(fmd->transitionTime);
        
        //draw cursor
        if(focusedItem == 1) drawCursor(0, 9);
        else drawCursor(58, (focusedItem * 9) + 2);
        
        changeFocusOnUpDown(4);
        toggleEditModeOnSelectClicked();
        
        //edit
        if(focusedItem == 1)
          thisFmdIdx = incDec(thisFmdIdx, 0, NUM_FLIGHT_MODES - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 2 && isEditMode)
          isEditTextDialog = true;
        else if(focusedItem == 3 && thisFmdIdx > 0 && isEditMode)
          fmd->swtch = incDecControlSwitch(fmd->swtch);
        else if(focusedItem == 4)
          fmd->transitionTime = incDec(fmd->transitionTime, 0, 50, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
  }
}

#endif
