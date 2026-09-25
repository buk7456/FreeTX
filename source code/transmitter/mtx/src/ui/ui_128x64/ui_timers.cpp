#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_timers()
{
  switch(theScreen)
  {
    case SCREEN_TIMERS:
      {
        timer_params_t *tmr = &Model.Timer[thisTimerIdx]; 
        
        if(isEditTextDialog) 
        {
          editTextDialog(PSTR("Timer name"), tmr->name, sizeof(tmr->name), true, true, false);
          break;
        }
        
        drawHeader(extrasMenu[EXTRAS_MENU_TIMERS]);

        display.setCursor(8, 9);
        getSrcName(textBuff, SRC_TIMER_FIRST + thisTimerIdx, sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 8, BLACK);

        display.setCursor(0, 20);
        display.print(F("Name:"));
        display.setCursor(60, 20);
        if(isEmptyStr(tmr->name, sizeof(tmr->name)))
          display.print(F("--"));
        else
          display.print(tmr->name);

        display.setCursor(0, 29);
        display.print(F("Switch:"));
        display.setCursor(60, 29);
        getControlSwitchName(textBuff, tmr->swtch, sizeof(textBuff));
        display.print(textBuff);
        
        display.setCursor(0, 38);
        display.print(F("Reset:"));
        display.setCursor(60, 38);
        getControlSwitchName(textBuff, tmr->resetSwitch, sizeof(textBuff));
        display.print(textBuff);
        
        display.setCursor(0, 47);
        display.print(F("Initial:"));
        display.setCursor(60, 47);
        if(tmr->initialSeconds == 0)
          display.print(F("--"));
        else
          printHHMMSS((uint32_t) tmr->initialSeconds * 1000);
        
        display.setCursor(0, 56);
        display.print(F("Persist:"));
        drawCheckbox(60, 56, tmr->isPersistent);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == 7 ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);
        
        //Show the value of the timer, right aligned.
        //Here we are employing a hack to measure the width of the text 
        //by intentionally printing off screen and using the difference in the x position.
        display.setCursor(0, 64);
        printTimerValue(thisTimerIdx);
        uint8_t len_px = display.getCursorX();
        //now print on screen
        uint8_t xpos = 127 - len_px;
        display.setCursor(xpos, 9);
        printTimerValue(thisTimerIdx);
        display.drawRect(xpos - 2, 7, len_px + 3, 11, BLACK);
        
        //show cursor
        if(focusedItem == 1)
          drawCursor(0, 9);
        else if(focusedItem <= 6)
          drawCursor(52, 20 + 9 * (focusedItem - 2));
        
        //handle navigation
        static uint8_t lastFocusedItem = 1;
        focusedItem = lastFocusedItem;
        changeFocusOnUpDown(7);
        toggleEditModeOnSelectClicked();
        lastFocusedItem = focusedItem;
        
        //edit items
        if(focusedItem == 1)
          thisTimerIdx = incDec(thisTimerIdx, 0, NUM_TIMERS - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 2 && isEditMode)
          isEditTextDialog = true;
        else if(focusedItem == 3 && isEditMode)
          tmr->swtch = incDecControlSwitch(tmr->swtch);
        else if(focusedItem == 4 && isEditMode)
          tmr->resetSwitch = incDecControlSwitch(tmr->resetSwitch);
        else if(focusedItem == 5 && isEditMode)
          changeToScreen(DIALOG_TIMER_INITIAL_TIME);
        else if(focusedItem == 6)
          tmr->isPersistent = incDec(tmr->isPersistent, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 7 && isEditMode)
        {
          changeToScreen(CONTEXT_MENU_TIMERS);
          lastFocusedItem = 1;
        }

        //exit 
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_EXTRAS_MENU);
          lastFocusedItem = 1;
        }
      }
      break;

    case DIALOG_TIMER_INITIAL_TIME:
      {
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Initial time"));

        static bool initialised = false;

        static uint8_t idxQQ = 0;
        static uint8_t valQQ[3];

        if(!initialised)
        {
          initialised = true;

          uint32_t hh, mm, ss;
          ss = Model.Timer[thisTimerIdx].initialSeconds;
          hh = ss / 3600;
          ss = ss - hh * 3600;
          mm = ss / 60;
          ss = ss - mm * 60;

          valQQ[0] = hh;
          valQQ[1] = mm;
          valQQ[2] = ss;
        }

        isEditMode = true;
        valQQ[idxQQ] = incDec(valQQ[idxQQ], 0, (idxQQ == 0) ? 23 : 59, INCDEC_WRAP, INCDEC_SLOW, INCDEC_NORMAL);

        //draw values
        display.setCursor(41, 32);
        for(uint8_t i = 0; i < sizeof(valQQ)/sizeof(valQQ[0]); i++)
        {
          if(i != 0)
            display.print(F(":"));
          if(valQQ[i] < 10)
            display.print(F("0"));
          display.print(valQQ[i]);
        }

        //draw blinking cursor
        if((millis() - buttonReleaseTime) % 1000 < 500 || buttonCode > 0)
          display.fillRect(41 + idxQQ * 18, 40, 11, 2, BLACK);

        //change to next
        if(clickedButton == KEY_SELECT)
          idxQQ++;
        else if(heldButton == KEY_SELECT && idxQQ > 0)
        {
          //move to previous
          idxQQ--;
          killButtonEvents();
        }

        //clear values
        if(idxQQ == 0 && heldButton == KEY_SELECT && millis() - buttonStartTime >= 1000)
        {
          killButtonEvents();
          valQQ[0] = 0;
          valQQ[1] = 0;
          valQQ[2] = 0;
        }

        //done, exit
        if(idxQQ == sizeof(valQQ)/sizeof(valQQ[0]))
        {
          idxQQ = 0;
          Model.Timer[thisTimerIdx].initialSeconds = ((uint32_t) 3600 * valQQ[0]) + ((uint32_t) 60 * valQQ[1]) + valQQ[2];
          initialised = false;
          changeToScreen(SCREEN_TIMERS);
        }
      }
      break;
      
    case CONTEXT_MENU_TIMERS:
      {
        enum {
          ITEM_START_STOP_TIMER,
          ITEM_RESET_TIMER,
          ITEM_RESET_SETTINGS,
        };
        
        contextMenuInitialise();
        if(Model.Timer[thisTimerIdx].swtch == CTRL_SW_NONE)
        {
          if(timerIsRunning[thisTimerIdx]) 
            contextMenuAddItem(PSTR("Stop timer"), ITEM_START_STOP_TIMER);
          else 
            contextMenuAddItem(PSTR("Start timer"), ITEM_START_STOP_TIMER);
        }
        contextMenuAddItem(PSTR("Reset timer"), ITEM_RESET_TIMER);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_START_STOP_TIMER)
        {
          timerForceRun[thisTimerIdx] = timerIsRunning[thisTimerIdx] ? false : true;
          changeToScreen(SCREEN_TIMERS);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_TIMER)
        {
          resetTimerRegister(thisTimerIdx);
          changeToScreen(SCREEN_TIMERS);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetTimerParams(thisTimerIdx);
          resetTimerRegister(thisTimerIdx); //also reset the register
          changeToScreen(SCREEN_TIMERS);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TIMERS);
      }
      break;
  }
}

#endif
