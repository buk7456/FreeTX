#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_counters()
{
  switch(theScreen)
  {
    case SCREEN_COUNTERS:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_COUNTERS]);
        
        counter_params_t *counter = &Model.Counter[thisCounterIdx];
        
        display.setCursor(8, 9);
        getSrcName(textBuff, SRC_COUNTER_FIRST + thisCounterIdx, sizeof(textBuff));
        display.print(textBuff);
        if(!isEmptyStr(counter->name, sizeof(counter->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(counter->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //show the value of the counter. Right align.
        //using a hack to measure the width of the text before actually printing.
        display.setCursor(0, 64);
        display.print(counterOut[thisCounterIdx]);
        uint8_t len_px = display.getCursorX();
        //now print on screen
        uint8_t xpos = 127 - len_px;
        display.setCursor(xpos, 9);
        display.print(counterOut[thisCounterIdx]);
        display.drawRect(xpos - 2, 7, len_px + 3, 11, BLACK);
        
        //--- scrollable list 
  
        enum {
          ITEM_CLOCK,
          ITEM_EDGE,
          ITEM_INCREMENT_CLOCK,
          ITEM_INCREMENT_EDGE,
          ITEM_DECREMENT_CLOCK,
          ITEM_DECREMENT_EDGE,
          ITEM_CLEAR,
          ITEM_MODULUS,
          ITEM_DIRECTION,
          ITEM_ROLLOVER_ENABLED,
          ITEM_PERSISTENT,
          
          ITEM_COUNT
        };

        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of IDs
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(counter->type == COUNTER_TYPE_BASIC)
          {
            if(i == ITEM_INCREMENT_CLOCK || i == ITEM_DECREMENT_CLOCK || i == ITEM_INCREMENT_EDGE || i == ITEM_DECREMENT_EDGE)
              continue;
          }
          else if(counter->type == COUNTER_TYPE_ADVANCED)
          {
            if(i == ITEM_DIRECTION || i == ITEM_CLOCK || i == ITEM_EDGE)
              continue;
          }

          listItemIDs[listItemCount++] = i;
        }

        //handle navigation
        uint8_t numFocusable = listItemCount + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        
        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < ITEM_COUNT; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(64, ypos);

          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_CLOCK:
              {
                display.print(F("Clock:"));
                display.setCursor(72, ypos);
                getControlSwitchName(textBuff, counter->clock, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  counter->clock = incDecControlSwitch(counter->clock);
              }
              break;
              
            case ITEM_INCREMENT_CLOCK:
              {
                display.print(F("Inc clock:"));
                display.setCursor(72, ypos);
                getControlSwitchName(textBuff, counter->incrementClock, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  counter->incrementClock = incDecControlSwitch(counter->incrementClock);
              }
              break;
              
            case ITEM_DECREMENT_CLOCK:
              {
                display.print(F("Dec clock:"));
                display.setCursor(72, ypos);
                getControlSwitchName(textBuff, counter->decrementClock, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  counter->decrementClock = incDecControlSwitch(counter->decrementClock);
              }
              break;
              
            case ITEM_EDGE:
              {
                display.print(F("Edge:"));
                display.setCursor(72, ypos);
                display.print(findStringInIdStr(enum_ClockEdge, counter->edge));
                if(edit)
                  counter->edge = incDec(counter->edge, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_INCREMENT_EDGE:
              {
                display.print(F("Inc edge:"));
                display.setCursor(72, ypos);
                display.print(findStringInIdStr(enum_ClockEdge, counter->incrementEdge));
                if(edit)
                  counter->incrementEdge = incDec(counter->incrementEdge, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_DECREMENT_EDGE:
              {
                display.print(F("Dec edge:"));
                display.setCursor(72, ypos);
                display.print(findStringInIdStr(enum_ClockEdge, counter->decrementEdge));
                if(edit)
                  counter->decrementEdge = incDec(counter->decrementEdge, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_MODULUS:
              {
                display.print(F("Modulus:"));
                display.setCursor(72, ypos);
                display.print(counter->modulus);
                if(edit)
                  counter->modulus = incDec(counter->modulus, 2, 10000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;
            
            case ITEM_DIRECTION:
              {
                display.print(F("Direction:"));
                display.setCursor(72, ypos);
                display.print(findStringInIdStr(enum_CounterDirection, counter->direction));
                if(edit)
                  counter->direction = incDec(counter->direction, 0, 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_CLEAR:
              {
                display.print(F("Clear:"));
                display.setCursor(72, ypos);
                getControlSwitchName(textBuff, counter->clear, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  counter->clear = incDecControlSwitch(counter->clear);
              }
              break;
              
            case ITEM_ROLLOVER_ENABLED:
              {
                display.print(F("Rollover:"));
                drawCheckbox(72, ypos, counter->rolloverEnabled);
                if(edit)
                  counter->rolloverEnabled = incDec(counter->rolloverEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
              
            case ITEM_PERSISTENT:
              {
                display.print(F("Persist:"));
                drawCheckbox(72, ypos, counter->isPersistent);
                if(edit)
                  counter->isPersistent = incDec(counter->isPersistent, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_COUNTERS);
        
        //change to next counter
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisCounterIdx = incDec(thisCounterIdx, 0, NUM_COUNTERS - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
      
    case CONTEXT_MENU_COUNTERS:
      {
        enum {
          ITEM_RENAME_COUNTER,
          ITEM_COUNTER_TYPE,
          ITEM_RESET_SETTINGS,
          ITEM_COPY_COUNTER,
          ITEM_CLEAR_COUNTER,
          ITEM_CLEAR_ALL_COUNTERS,
          ITEM_VIEW_OUTPUTS,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_VIEW_OUTPUTS);
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_COUNTER);
        contextMenuAddItem(PSTR("Rename counter"), ITEM_RENAME_COUNTER);
        contextMenuAddItem(PSTR("Counter type"), ITEM_COUNTER_TYPE);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuAddItem(PSTR("Clear counter"), ITEM_CLEAR_COUNTER);
        contextMenuAddItem(PSTR("Clear all counters"), ITEM_CLEAR_ALL_COUNTERS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetCounterParams(thisCounterIdx);
          resetCounterRegister(thisCounterIdx); //also reset the register
          changeToScreen(SCREEN_COUNTERS);
        }
        if(contextMenuSelectedItemID == ITEM_CLEAR_COUNTER)
        {
          resetCounterRegister(thisCounterIdx);
          changeToScreen(SCREEN_COUNTERS);  
        }
        if(contextMenuSelectedItemID == ITEM_CLEAR_ALL_COUNTERS)
          changeToScreen(CONFIRMATION_CLEAR_ALL_COUNTERS);  
        if(contextMenuSelectedItemID == ITEM_RENAME_COUNTER)
          changeToScreen(DIALOG_RENAME_COUNTER);
        if(contextMenuSelectedItemID == ITEM_COUNTER_TYPE)
          changeToScreen(DIALOG_COUNTER_TYPE);
        if(contextMenuSelectedItemID == ITEM_COPY_COUNTER)
        {
          destCounterIdx = thisCounterIdx;
          changeToScreen(DIALOG_COPY_COUNTER);  
        }
        if(contextMenuSelectedItemID == ITEM_VIEW_OUTPUTS)
          changeToScreen(SCREEN_COUNTER_OUTPUTS);

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_COUNTERS);
      }
      break;
      
    case DIALOG_RENAME_COUNTER:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Counter name"), Model.Counter[thisCounterIdx].name, sizeof(Model.Counter[0].name), 
                       true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_COUNTERS);
      }
      break;
      
    case DIALOG_COPY_COUNTER:
      {
        isEditMode = true;
        destCounterIdx = incDec(destCounterIdx, 0, NUM_COUNTERS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Counter"), thisCounterIdx, destCounterIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.Counter[destCounterIdx] = Model.Counter[thisCounterIdx];
          thisCounterIdx = destCounterIdx;
          changeToScreen(SCREEN_COUNTERS);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_COUNTERS);
      }
      break;

    case DIALOG_COUNTER_TYPE:
      {
        isEditMode = true;
        Model.Counter[thisCounterIdx].type = incDec(Model.Counter[thisCounterIdx].type, 0, COUNTER_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);

        drawBoundingBox(11, 14, 105, 35,BLACK);
        drawCursor(21, 28);
        display.setCursor(17, 18);
        display.print(F("Counter type"));
        display.setCursor(29, 28);
        display.print(findStringInIdStr(enum_CounterType, Model.Counter[thisCounterIdx].type));
        
        if(heldButton == KEY_SELECT || clickedButton == KEY_SELECT ) //exit
          changeToScreen(SCREEN_COUNTERS);
      }
      break;
      
    case CONFIRMATION_CLEAR_ALL_COUNTERS:
      {
        printFullScreenMessage(PSTR("Clear all counter\nregisters?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          resetCounterRegisters();
          changeToScreen(SCREEN_COUNTERS);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_COUNTERS);
      }
      break;

    case SCREEN_COUNTER_OUTPUTS:
      {
        drawHeader(PSTR("Counter outputs"));

        //--- scrollable 
        
        uint8_t numPages = (NUM_COUNTERS + 4) / 5;
        static uint8_t thisPage = 1;
        
        static bool viewInitialised = false;
        if(!viewInitialised) 
        {
          thisPage = (thisCounterIdx + 5) / 5; //start in the page with the counter we want to view
          viewInitialised = true;
        }

        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint8_t startIdx = (thisPage - 1) * 5;
        for(uint8_t i = startIdx; i < startIdx + 5 && i < NUM_COUNTERS; i++)
        {
          uint8_t ypos = 10 + (i - startIdx) * 11;
          //show marker
          if(i == thisCounterIdx) 
          {
            display.setCursor(0, ypos);
            display.write(0xB1);
          }
          //show name
          display.setCursor(7, ypos);
          display.print(F("Counter"));
          display.print(i + 1);
          display.print(F(":"));
          //show value
          display.setCursor(73, ypos);
          display.print(counterOut[i]);
        }
        
        //show scrollbar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        if(heldButton == KEY_SELECT) //exit
        {
          viewInitialised = false;
          changeToScreen(SCREEN_COUNTERS);
        }
      }
      break;
  }
}

#endif
