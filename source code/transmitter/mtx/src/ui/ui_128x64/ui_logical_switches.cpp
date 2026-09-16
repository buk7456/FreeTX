#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_logical_switches()
{
  switch(theScreen)
  {
    case SCREEN_LOGICAL_SWITCHES:
      {
        //intelligently disable screen interlacing
        static bool lastState;
        bool state = checkSwitchCondition(CTRL_SW_LOGICAL_FIRST + thisLsIdx);
        if(state != lastState)
        {
          lastState = state;
          display.setInterlace(false);
        }

        drawHeader(extrasMenu[EXTRAS_MENU_LOGICAL_SWITCHES]);

        logical_switch_t *ls = &Model.LogicalSwitch[thisLsIdx];
        
        //draw title
        display.setCursor(8, 9);
        getSrcName(textBuff, SRC_SW_LOGICAL_FIRST + thisLsIdx, sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //draw switch icon
        if(checkSwitchCondition(CTRL_SW_LOGICAL_FIRST + thisLsIdx))
          display.drawBitmap(115, 9, icon_switch_is_on, 13, 8, BLACK);
        else
          display.drawBitmap(115, 9, icon_switch_is_off, 13, 8, BLACK);
        
        //-- dynamic scrollable list
        
        enum {
          ITEM_LS_FUNC,
          ITEM_LS_VAL1,
          ITEM_LS_VAL2,
          ITEM_LS_VAL3,
          ITEM_LS_VAL4,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(i == ITEM_LS_VAL4 && (ls->func == LS_FUNC_LATCH || ls->func == LS_FUNC_TOGGLE || ls->func == LS_FUNC_PULSE))
            continue;
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
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(52, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_LS_FUNC:
              {
                display.print(F("Functn:"));
                display.setCursor(60, ypos);
                if(ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
                  display.print(F("|\xEB|>x"));
                else
                  display.print(findStringInIdStr(enum_LogicalSwitch_Func, ls->func));
                
                if(edit)
                {
                  uint8_t oldGroup = getLSFuncGroup(ls->func);
                  ls->func = incDec(ls->func, 0, LS_FUNC_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  uint8_t newGroup = getLSFuncGroup(ls->func);
                  if(newGroup != oldGroup) //reset values if group changed
                  {
                    ls->val3 = 0;
                    ls->val4 = 0;
                    if(ls->func <= LS_FUNC_GROUP3_LAST)     
                    {
                      ls->val1 = SRC_NONE; 
                      ls->val2 = 0; 
                      if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X) 
                        ls->val3 = 2;
                    }
                    else if(ls->func <= LS_FUNC_GROUP4_LAST)
                    {
                      ls->val1 = SRC_NONE; 
                      ls->val2 = SRC_NONE;
                    }
                    else if(ls->func <= LS_FUNC_GROUP5_LAST)
                    {
                      ls->val1 = CTRL_SW_NONE; 
                      ls->val2 = CTRL_SW_NONE;
                    }
                    else if(ls->func == LS_FUNC_TOGGLE) 
                    {
                      ls->val1 = CTRL_SW_NONE; 
                      ls->val2 = 0; 
                      ls->val3 = CTRL_SW_NONE;
                    }
                    else if(ls->func == LS_FUNC_PULSE)
                    {
                      ls->val1 = 5; 
                      ls->val2 = 10;
                    }
                  }
                }
              }
              break;
              
            case ITEM_LS_VAL1:
              {
                if(ls->func == LS_FUNC_PULSE) display.print(F("Width:"));
                else if(ls->func == LS_FUNC_LATCH) display.print(F("Set:"));
                else if(ls->func == LS_FUNC_TOGGLE) display.print(F("Clock:"));
                else display.print(F("Value1:"));
                
                display.setCursor(60, ypos);
                
                if(ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else if(ls->func <= LS_FUNC_GROUP4_LAST)
                {
                  if(ls->val1 == SRC_NONE)
                    display.print(F("--"));
                  else
                  {
                    getSrcName(textBuff, ls->val1, sizeof(textBuff));
                    display.print(textBuff);
                  }
                }
                else if(ls->func <= LS_FUNC_GROUP5_LAST || ls->func == LS_FUNC_TOGGLE)
                {
                  getControlSwitchName(textBuff, ls->val1, sizeof(textBuff));
                  display.print(textBuff);
                }
                else if(ls->func == LS_FUNC_PULSE)
                  printSeconds(ls->val1);
                
                if(edit && ls->func != LS_FUNC_NONE)
                {
                  if(ls->func <= LS_FUNC_GROUP3_LAST)
                  {
                    //auto detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      ls->val1 = movedSrc;
                    //inc dec
                    ls->val1 = incDecSource(ls->val1, INCDEC_FLAG_MIX_SRC 
                                                    | INCDEC_FLAG_COUNTER_AS_SRC 
                                                    | INCDEC_FLAG_TIMER_AS_SRC
                                                    | INCDEC_FLAG_TELEM_AS_SRC 
                                                    | INCDEC_FLAG_INACTIVITY_TIMER_AS_SRC 
                                                    | INCDEC_FLAG_TX_BATTV_AS_SRC);
                    //reset ls->val2 if out of range
                    if(ls->val1 < MIX_SOURCES_COUNT)
                    {
                      if(ls->val2 > 100 || ls->val2 < -100)
                        ls->val2 = 0;
                    }
                    else if(ls->val1 >= SRC_COUNTER_FIRST && ls->val1 <= SRC_COUNTER_LAST)
                    {
                      if(ls->val2 > 9999 || ls->val2 < 0)
                        ls->val2 = 0;
                    }
                    else if(ls->val1 == SRC_INACTIVITY_TIMER || ls->val1 == SRC_TX_BATTERY_VOLTAGE)
                    {
                      if(ls->val2 < 0)
                        ls->val2 = 0;
                    }
                  }
                  else if(ls->func <= LS_FUNC_GROUP4_LAST)
                  {
                    //auto detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      ls->val1 = movedSrc;
                    //inc dec
                    ls->val1 = incDecSource(ls->val1, INCDEC_FLAG_MIX_SRC);
                  }
                  else if(ls->func <= LS_FUNC_GROUP5_LAST || ls->func == LS_FUNC_TOGGLE)
                    ls->val1 = incDecControlSwitch(ls->val1);
                  else if(ls->func == LS_FUNC_PULSE)
                    ls->val1 = incDec(ls->val1, 1, ls->val2 - 1, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
            
            case ITEM_LS_VAL2:
              {
                if(ls->func == LS_FUNC_PULSE) display.print(F("Period:"));
                else if(ls->func == LS_FUNC_LATCH) display.print(F("Reset:"));
                else if(ls->func == LS_FUNC_TOGGLE) display.print(F("Edge:"));
                else display.print(F("Value2:"));
                
                display.setCursor(60, ypos);
                if(ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else if(ls->func <= LS_FUNC_GROUP3_LAST)
                {
                  if(ls->val1 < MIX_SOURCES_COUNT)
                    display.print(ls->val2);
                  else if(ls->val1 >= SRC_COUNTER_FIRST && ls->val1 <= SRC_COUNTER_LAST)
                    display.print(ls->val2);
                  else if(ls->val1 >= SRC_TIMER_FIRST && ls->val1 <= SRC_TIMER_LAST)
                    printHHMMSS((int32_t)ls->val2 * 1000);
                  else if(ls->val1 >= SRC_TELEMETRY_FIRST && ls->val1 <= SRC_TELEMETRY_LAST)
                  {
                    display.setCursor(display.getCursorX(), display.getCursorY());
                    printTelemParam(ls->val1 - SRC_TELEMETRY_FIRST, ls->val2, true);
                  }
                  else if(ls->val1 == SRC_INACTIVITY_TIMER)
                    printHHMMSS((int32_t) ls->val2 * 1000);
                  else if(ls->val1 == SRC_TX_BATTERY_VOLTAGE)
                    printVoltage(ls->val2);
                }
                else if(ls->func <= LS_FUNC_GROUP4_LAST)
                {
                  if(ls->val2 == SRC_NONE)
                    display.print(F("--"));
                  else
                  {
                    getSrcName(textBuff, ls->val2, sizeof(textBuff));
                    display.print(textBuff);
                  }
                }
                else if(ls->func <= LS_FUNC_GROUP5_LAST)
                {
                  getControlSwitchName(textBuff, ls->val2, sizeof(textBuff));
                  display.print(textBuff);
                }
                else if(ls->func == LS_FUNC_TOGGLE)
                  display.print(findStringInIdStr(enum_ClockEdge, ls->val2));
                else if(ls->func == LS_FUNC_PULSE)
                  printSeconds(ls->val2);

                if(edit && ls->func != LS_FUNC_NONE)
                {
                  if(ls->func <= LS_FUNC_GROUP3_LAST)
                  {
                    if(ls->val1 < MIX_SOURCES_COUNT)
                      ls->val2 = incDec(ls->val2, (ls->func <= LS_FUNC_GROUP1_LAST) ? -100 : 0, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                    else if(ls->val1 >= SRC_COUNTER_FIRST && ls->val1 <= SRC_COUNTER_LAST)
                      ls->val2 = incDec(ls->val2, 0, 9999, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                    else if(ls->val1 >= SRC_TIMER_FIRST && ls->val1 <= SRC_TIMER_LAST)
                      ls->val2 = incDec(ls->val2, (ls->func <= LS_FUNC_GROUP1_LAST) ? -30000 : 0, 30000, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_FAST);
                    else if(ls->val1 >= SRC_TELEMETRY_FIRST && ls->val1 <= SRC_TELEMETRY_LAST)
                      ls->val2 = incDec(ls->val2, (ls->func <= LS_FUNC_GROUP1_LAST) ? -30000 : 0, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                    else if(ls->val1 == SRC_INACTIVITY_TIMER)
                      ls->val2 = incDec(ls->val2, 0, 30000, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_FAST);
                    else if(ls->val1 == SRC_TX_BATTERY_VOLTAGE)
                    {
                      ls->val2 /= 10;
                      ls->val2 = incDec(ls->val2, 0, 3000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                      ls->val2 *= 10;
                    }
                  }
                  else if(ls->func <= LS_FUNC_GROUP4_LAST)
                  {
                    //auto detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      ls->val2 = movedSrc;
                    //inc dec
                    ls->val2 = incDecSource(ls->val2, INCDEC_FLAG_MIX_SRC);
                  }
                  else if(ls->func <= LS_FUNC_GROUP5_LAST)
                    ls->val2 = incDecControlSwitch(ls->val2);
                  else if(ls->func == LS_FUNC_TOGGLE) //change edge type
                    ls->val2 = incDec(ls->val2, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
                  else if(ls->func == LS_FUNC_PULSE) //adjust period
                    ls->val2 = incDec(ls->val2, ls->val1 + 1, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
              
            case ITEM_LS_VAL3:
              {
                if(ls->func == LS_FUNC_TOGGLE)
                {
                  display.print(F("Clear:"));
                  display.setCursor(60, ypos);
                  getControlSwitchName(textBuff, ls->val3, sizeof(textBuff));
                  display.print(textBuff);
                }
                else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
                {
                  display.print(F("Dirctn:"));
                  display.setCursor(60, ypos);
                  display.print(findStringInIdStr(enum_DirectionOfChange, ls->val3));
                }
                else
                {
                  display.print(F("Delay:"));
                  display.setCursor(60, ypos);
                  if(ls->val3 == 0 || ls->func == LS_FUNC_NONE)
                    display.print(F("--"));
                  else
                    printSeconds(ls->val3);
                }

                if(edit && ls->func != LS_FUNC_NONE)
                {
                  if(ls->func == LS_FUNC_TOGGLE)
                    ls->val3 = incDecControlSwitch(ls->val3);
                  else if(ls->func == LS_FUNC_ABS_DELTA_GREATER_THAN_X)
                    ls->val3 = incDec(ls->val3, 0, 2, INCDEC_WRAP, INCDEC_SLOW);
                  else //adjust delay
                    ls->val3 = incDec(ls->val3, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
              
            case ITEM_LS_VAL4:
              {
                display.print(F("Duratn:"));
                display.setCursor(60, ypos);
                if(ls->val4 == 0 || ls->func == LS_FUNC_NONE)
                  display.print(F("--"));
                else
                  printSeconds(ls->val4);
                
                if(edit && ls->func != LS_FUNC_NONE)
                  ls->val4 = incDec(ls->val4, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
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
          changeToScreen(CONTEXT_MENU_LOGICAL_SWITCHES);
        
        //change to next logical switch
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisLsIdx = incDec(thisLsIdx, 0, NUM_LOGICAL_SWITCHES - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
      
    case CONTEXT_MENU_LOGICAL_SWITCHES:
      {
        enum {
          ITEM_VIEW_OUTPUTS,
          ITEM_COPY_TO,
          ITEM_MOVE_TO,
          ITEM_RESET_SETTINGS
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_VIEW_OUTPUTS);
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_TO);
        contextMenuAddItem(PSTR("Move to"), ITEM_MOVE_TO);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_VIEW_OUTPUTS)
          changeToScreen(SCREEN_LOGICAL_SWITCH_OUTPUTS);
        if(contextMenuSelectedItemID == ITEM_COPY_TO)
        {
          destLsIdx = thisLsIdx;
          changeToScreen(DIALOG_COPY_LOGICAL_SWITCH);
        }
        if(contextMenuSelectedItemID == ITEM_MOVE_TO)
        {
          destLsIdx = thisLsIdx;
          changeToScreen(DIALOG_MOVE_LOGICAL_SWITCH);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetLogicalSwitchParams(thisLsIdx);
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
      }
      break;
    
    case DIALOG_COPY_LOGICAL_SWITCH:
    case DIALOG_MOVE_LOGICAL_SWITCH:
      {
        isEditMode = true;
        destLsIdx = incDec(destLsIdx, 0, NUM_LOGICAL_SWITCHES - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("L"), thisLsIdx, destLsIdx, theScreen == DIALOG_COPY_LOGICAL_SWITCH);
        if(clickedButton == KEY_SELECT)
        {
          if(theScreen == DIALOG_COPY_LOGICAL_SWITCH)
          {
            Model.LogicalSwitch[destLsIdx] = Model.LogicalSwitch[thisLsIdx];
            thisLsIdx = destLsIdx;
          }
          else
          {
            if(moveLogicalSwitch(destLsIdx, thisLsIdx))
              thisLsIdx = destLsIdx;
            else
              makeToast(PSTR("Moving failed"), 2000, 0);
          }
          
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
      }
      break;

    case SCREEN_LOGICAL_SWITCH_OUTPUTS:
      {
        display.setInterlace(false); 
        drawHeader(PSTR("Logical sw outputs"));
        
        //--- scrollable 
        
        uint8_t numPages = (NUM_LOGICAL_SWITCHES + 9) / 10;
        static uint8_t thisPage = 1;
        
        static bool viewInitialised = false;
        if(!viewInitialised) 
        {
          //start in the page that has the switch we want to view
          thisPage = (thisLsIdx + 10) / 10;
          viewInitialised = true;
        }

        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint8_t startIdx = (thisPage - 1) * 10;
        for(uint8_t i = startIdx; i < startIdx + 10 && i < NUM_LOGICAL_SWITCHES; i++)
        {
          uint8_t xpos, ypos;
          if((i - startIdx) < 5)
          {
            xpos = 6;
            ypos = 10 + (i - startIdx) * 11;
          }
          else
          { 
            xpos = 71;
            ypos = 10 + (i - (startIdx + 5)) * 11;
          }
          //show marker
          if(i == thisLsIdx)
          {
            display.setCursor(xpos - 6, ypos);
            display.write(0xB1);
          }
          //show name
          display.setCursor(xpos, ypos);
          getSrcName(textBuff, SRC_SW_LOGICAL_FIRST + i, sizeof(textBuff));
          display.print(textBuff);
          display.print(F(":"));
          //draw the switch state icon
          if(checkSwitchCondition(CTRL_SW_LOGICAL_FIRST + i))
            display.drawBitmap(xpos + 24, ypos, icon_switch_is_on, 13, 8, BLACK);
          else
            display.drawBitmap(xpos + 24, ypos, icon_switch_is_off, 13, 8, BLACK);
        }

        //show scrollbar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        //exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_LOGICAL_SWITCHES);
        }
      }
      break;
  }
}

#endif
