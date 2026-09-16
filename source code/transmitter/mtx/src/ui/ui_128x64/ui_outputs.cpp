#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_outputs()
{
  switch(theScreen)
  {
    case SCREEN_OUTPUTS:
      {
        channel_params_t *ch = &Model.Channel[thisChIdx];

        drawHeader(mainMenu[MAIN_MENU_OUTPUTS]);
        
        display.setCursor(8, 9);
        getSrcName(textBuff, SRC_CH1 + thisChIdx, sizeof(textBuff));
        display.print(textBuff);
        if(!isEmptyStr(ch->name, sizeof(ch->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(ch->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //--- scrollable list 
        
        enum {
          ITEM_REVERSE,
          ITEM_SUBTRIM,
          ITEM_OVERRIDE_SWITCH,
          ITEM_OVERRIDE_VALUE,
          ITEM_FAILSAFE,
          ITEM_ENDPOINT_L,
          ITEM_ENDPOINT_R,
          ITEM_CURVE,
          
          ITEM_COUNT
        };
        
        static const uint8_t qqTab[ITEM_COUNT][2] PROGMEM = {
          //{Item, Vertical offset} 
          //Items should be ordered as in the enumeration above 
          {ITEM_REVERSE, 0},
          {ITEM_SUBTRIM, 1},
          {ITEM_OVERRIDE_SWITCH, 2},
          {ITEM_OVERRIDE_VALUE, 2},
          {ITEM_FAILSAFE, 3},
          {ITEM_ENDPOINT_L, 4},
          {ITEM_ENDPOINT_R, 4},
          {ITEM_CURVE, 5}
        };
        
        const uint8_t totalLines = 6;
        const uint8_t maxVisibleLines = 5;
        
        static uint8_t topLine = 0;
        
        //handle navigation
        uint8_t numFocusable = ITEM_COUNT + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topLine = 0;
        else if(focusedItem > 1)
        {
          uint8_t verticalOffset = pgm_read_byte(&(qqTab[focusedItem - 2][1]));
          if(verticalOffset < topLine)
            topLine = verticalOffset;
          while(verticalOffset >= topLine + maxVisibleLines)
            topLine++;
        }

        //fill list and edit items
        uint8_t itemID = topLine;
        while(itemID < ITEM_COUNT)
        {
          bool isFocused = (focusedItem > 1 && focusedItem != numFocusable && itemID == focusedItem - 2);
          bool edit = (isFocused && isEditMode);
          
          uint8_t ypos = 20 + (pgm_read_byte(&(qqTab[itemID][1])) - topLine) * 9;
          if(ypos > (20 + (maxVisibleLines - 1) * 9))
            break;

          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_REVERSE:
              {
                display.print(F("Reverse:"));
                drawCheckbox(62, ypos, ch->reverse);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->reverse = incDec(ch->reverse, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_SUBTRIM:
              {
                display.print(F("Subtrim:"));
                display.setCursor(62, ypos);
                int16_t val = ch->subtrim;
                if(val < 0)
                {
                  display.print(F("-"));
                  val = -val;
                }
                display.print(val / 10);
                display.print(F("."));
                display.print(val % 10);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                {
                  //restrict to multiples of 2
                  int16_t val = ch->subtrim / 2;
                  val = incDec(val, TRIM_MIN_VAL / 2, TRIM_MAX_VAL / 2, INCDEC_NOWRAP, INCDEC_NORMAL);
                  ch->subtrim = val * 2;
                }
              }
              break;
            
            case ITEM_OVERRIDE_SWITCH:
              {
                display.print(F("Override:"));
                display.setCursor(62, ypos);
                getControlSwitchName(textBuff, ch->overrideSwitch, sizeof(textBuff));
                display.print(textBuff);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->overrideSwitch = incDecControlSwitch(ch->overrideSwitch);
              }
              break;
            
            case ITEM_OVERRIDE_VALUE:
              {
                display.setCursor(98, ypos);
                if(ch->overrideSwitch == CTRL_SW_NONE) 
                  display.print(F("--"));
                else 
                  display.print(ch->overrideVal);
                if(isFocused)
                  drawCursor(90, ypos);
                if(edit && ch->overrideSwitch != CTRL_SW_NONE)
                  ch->overrideVal = incDec(ch->overrideVal, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_FAILSAFE:
              {
                display.print(F("Failsafe:"));
                display.setCursor(62, ypos);
                if(ch->failsafe < -100)
                  display.print(findStringInIdStr(enum_ChannelFailsafe, ch->failsafe));
                else
                  display.print(ch->failsafe);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->failsafe = incDec(ch->failsafe, -102, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_ENDPOINT_L:
              {
                display.print(F("Endpts:"));
                display.setCursor(62, ypos);
                display.print(ch->endpointL);
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->endpointL = incDec(ch->endpointL, -100, 0, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
            
            case ITEM_ENDPOINT_R:
              {
                display.setCursor(98, ypos);
                display.print(ch->endpointR);
                if(isFocused)
                  drawCursor(90, ypos);
                if(edit)
                  ch->endpointR = incDec(ch->endpointR, 0, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_CURVE:
              {
                display.print(F("Curve:"));
                display.setCursor(62, ypos);
                if(ch->curve == -1)
                {
                  display.print(findStringInIdStr(enum_ChannelCurve, ch->curve));
                  // display.print(F("--"));
                }
                else
                {
                  if(isEmptyStr(Model.CustomCurve[ch->curve].name, sizeof(Model.CustomCurve[0].name)))
                  {
                    display.print(F("Crv"));
                    display.print(ch->curve + 1);
                  }
                  else
                    display.print(Model.CustomCurve[ch->curve].name);
                }
                if(isFocused)
                  drawCursor(54, ypos);
                if(edit)
                  ch->curve = incDec(ch->curve, -1, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
          }
          itemID++;
        }
        
        //scrollbar
        drawScrollBar(127, 19, totalLines, topLine + 1, maxVisibleLines, maxVisibleLines * 9);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);

        //open context menu
        if(focusedItem == numFocusable && isEditMode)
          changeToScreen(CONTEXT_MENU_OUTPUTS);
        
        //change to next
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisChIdx = incDec(thisChIdx, 0, NUM_RC_CHANNELS - 1, INCDEC_WRAP, INCDEC_SLOW); 
        }

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MAIN_MENU);
      }
      break;
 
    case CONTEXT_MENU_OUTPUTS:
      {
        enum {
          ITEM_CHANNEL_MONITOR,
          ITEM_RENAME_CHANNEL,
          ITEM_RESET_SETTINGS,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_CHANNEL_MONITOR);
        contextMenuAddItem(PSTR("Rename channel"), ITEM_RENAME_CHANNEL);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RENAME_CHANNEL)
          changeToScreen(DIALOG_RENAME_CHANNEL);
        if(contextMenuSelectedItemID == ITEM_CHANNEL_MONITOR)
          changeToScreen(SCREEN_CHANNEL_MONITOR);
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetChannelParams(thisChIdx);
          changeToScreen(SCREEN_OUTPUTS);
        }
        
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_OUTPUTS);
      }
      break;
      
    case DIALOG_RENAME_CHANNEL:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Channel name"), Model.Channel[thisChIdx].name, sizeof(Model.Channel[0].name), true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_OUTPUTS);
      }
      break;
  }
}

#endif
