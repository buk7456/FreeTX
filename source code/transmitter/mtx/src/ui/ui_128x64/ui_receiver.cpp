#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_receiver()
{
  switch(theScreen)
  {
    case SCREEN_RECEIVER:
      {
        drawHeader(mainMenu[MAIN_MENU_RECEIVER]);

        enum { 
          PAGE_MAIN_RECEIVER,
          PAGE_SECONDARY_RECEIVER,
          NUM_PAGES
        };
        
        uint8_t numPages = NUM_PAGES;
        if(MAX_CHANNELS_PER_RECEIVER == NUM_RC_CHANNELS)
          numPages = 1;
        
        static uint8_t page = PAGE_MAIN_RECEIVER;

        isMainReceiver = (page == PAGE_MAIN_RECEIVER);

        display.setCursor(8, 9);
        strlcpy_P(textBuff, isMainReceiver ? PSTR("Main rcvr") : PSTR("Secondary rcvr"), sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

        enum {
          ITEM_SECONDARY_RECEIVER_ENABLED,
          ITEM_BIND,
          ITEM_CONFIGURE,

          ITEM_COUNT
        };

        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(page == PAGE_MAIN_RECEIVER)
          {
            if(i == ITEM_SECONDARY_RECEIVER_ENABLED)
              continue;
          }
          else if(page == PAGE_SECONDARY_RECEIVER)
          {
            if(!Model.secondaryRcvrEnabled && i != ITEM_SECONDARY_RECEIVER_ENABLED)
              continue;
          }

          listItemIDs[listItemCount++] = i;
        }
        
        //initialise
        static uint8_t topItem;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount + 1); //+1 for title focus
        toggleEditModeOnSelectClicked();
        if(focusedItem == 1) //title focus
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
            drawCursor(0, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (focusedItem > 1 && itemID == listItemIDs[focusedItem - 2]);
          
          display.setCursor(8, ypos);
          switch(itemID)
          { 
            case ITEM_BIND:
              {
                display.print(F("[Bind]"));
                if(isFocused && clickedButton == KEY_SELECT)
                {
                  isRequestingBind = true;
                  changeToScreen(SCREEN_RECEIVER_BINDING);
                }
              }
              break;
            
            case ITEM_CONFIGURE:
              {
                display.print(F("[Configure]"));
                if(isFocused && clickedButton == KEY_SELECT)
                {
                  changeToScreen(SCREEN_RECEIVER_CONFIG);
                }
              }
              break;
              
            case ITEM_SECONDARY_RECEIVER_ENABLED:
              {
                drawCheckbox(8, ypos, Model.secondaryRcvrEnabled);
                display.setCursor(18, ypos);
                display.print(F("Enable"));
                if(isFocused && isEditMode)
                {
                  Model.secondaryRcvrEnabled = incDec(Model.secondaryRcvrEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);

        //change to next receiver
        if(focusedItem == 1)
        {
          drawCursor(0, 9);
          page = incDec(page, 0, numPages - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MAIN_MENU);
      }
      break;
      
    case SCREEN_RECEIVER_BINDING:
      {
        static uint32_t entryTime = 0;
        static bool initialised = false;
        if(!initialised)
        {
          initialised = true;
          entryTime = millis();
          bindStatusCode = 0;
        }

        printFullScreenMessage(PSTR("Binding"));
        if(Sys.animationsEnabled)
          drawLoaderSpinner(60, display.getCursorY() + 12, 2);
        
        if(bindStatusCode == 1)
        {
          makeToast(PSTR("Bind success"), 2000, 0);
          changeToScreen(SCREEN_RECEIVER);
          audioToPlay = AUDIO_BIND_SUCCESS;
          initialised = false;
        }
        else if(bindStatusCode == 2)
        {
          makeToast(PSTR("Bind failed"), 2000, 0);
          changeToScreen(SCREEN_RECEIVER);
          initialised = false;
        }
        else if(millis() - entryTime > 5000)
        {
          makeToast(PSTR("Bind timeout"), 2000, 0);
          changeToScreen(SCREEN_RECEIVER);
          initialised = false;
        }
      }
      break;
      
    case SCREEN_RECEIVER_CONFIG:
      {
        enum {
          QUERYING_CONFIG, 
          SENDING_CONFIG, 
          VIEWING_CONFIG,
          VIEWING_CONFIG_SERVOPWM
        };
        
        static uint8_t state = QUERYING_CONFIG;
        static bool stateInitialised = false;
        static uint32_t entryTime = 0;
        static bool actionStarted = false;
        
        if(!stateInitialised)
        {
          state = QUERYING_CONFIG;
          // state = VIEWING_CONFIG; //DEBUG
          entryTime = millis();
          gotOutputChConfig = false;
          stateInitialised = true;
          receiverConfigStatusCode = 0;
        }
        
        switch(state)
        {
          case QUERYING_CONFIG:
            {
              printFullScreenMessage(PSTR("Reading settings"));
              if(Sys.animationsEnabled)
                drawLoaderSpinner(60, display.getCursorY() + 12, 2);
              
              if(!actionStarted)
              {
                isRequestingOutputChConfig = true;
                actionStarted = true;
              }
              if(gotOutputChConfig)
              {
                actionStarted = false;
                state = VIEWING_CONFIG;
              }
              //Time out
              if(millis() - entryTime > 3000)
              {
                makeToast(PSTR("No response"), 2000, 0);
                stateInitialised = false;
                actionStarted = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
            
          case VIEWING_CONFIG:
            {
              drawHeader(PSTR("Rcvr output config"));

              display.setCursor(0, 9);
              display.print(F("Type of signal"));
            
              //--scrollable list--
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                isEditMode = false;
                viewInitialised = true;
              }
            
              uint8_t startIdx = 0; 
              uint8_t endIdx = MAX_CHANNELS_PER_RECEIVER - 1;
              
              //fill list
              uint8_t numItems = (endIdx - startIdx) + 1; 
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(34, ypos);
                
                display.setCursor(0, ypos);
                uint8_t idx = startIdx + item - 1; 
                if(idx <= endIdx)
                {
                  display.print(F("Ch"));
                  if(isMainReceiver)
                    display.print(idx + 1);
                  else
                    display.print(idx + 1 + MAX_CHANNELS_PER_RECEIVER);
                  display.print(F(":"));
                  display.setCursor(42, ypos);
                  uint8_t val = outputChConfig[idx] & 0x03;
                  display.print(findStringInIdStr(enum_OutputChConfig, val));
                }
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //show the write button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              display.setCursor(90, 56);
              display.print(F("[Next]"));
              if(focusedItem == numItems + 1)
                drawCursor(82, 56);
              
              //Handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
            
              //edit parameters
              uint8_t idx = startIdx + focusedItem - 1;
              if(idx <= endIdx)
              {
                uint8_t signalType = outputChConfig[idx] & 0x03;
                uint8_t maxSignalType = (outputChConfig[idx] >> 2) & 0x03;
                signalType = incDec(signalType, 0, maxSignalType, INCDEC_WRAP, INCDEC_SLOW);
                outputChConfig[idx] &= ~0x03;
                outputChConfig[idx] |= signalType;
              }
              
              //move to next 
              if(focusedItem == numItems + 1 && clickedButton == KEY_SELECT)
              {
                state = VIEWING_CONFIG_SERVOPWM;
                viewInitialised = false;
              }
              
              //exit
              if(heldButton == KEY_SELECT)
              {
                stateInitialised = false;
                actionStarted = false;
                viewInitialised = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
            
          case VIEWING_CONFIG_SERVOPWM:
            {
              drawHeader(PSTR("Rcvr output config"));

              display.setCursor(0, 9);
              display.print(F("Servo PWM range"));
            
              //--scrollable list--
              
              static uint8_t topItem;
              static bool viewInitialised = false;
              if(!viewInitialised)
              {
                focusedItem = 1;
                topItem = 1;
                isEditMode = false;
                viewInitialised = true;
              }
            
              uint8_t startIdx = 0; 
              uint8_t endIdx = MAX_CHANNELS_PER_RECEIVER - 1;
              
              //fill list
              uint8_t numItems = (endIdx - startIdx) + 1; 
              for(uint8_t line = 0; line < 4 && line < numItems; line++)
              {
                uint8_t ypos = 18 + line * 9;
                uint8_t item = topItem + line;
                if(focusedItem == item)
                  drawCursor(32, ypos);
                
                display.setCursor(0, ypos);
                uint8_t idx = startIdx + item - 1; 
                if(idx <= endIdx)
                {
                  display.print(F("Ch"));
                  if(isMainReceiver)
                    display.print(idx + 1);
                  else
                    display.print(idx + 1 + MAX_CHANNELS_PER_RECEIVER);
                  display.print(F(":"));
                  display.setCursor(40, ypos);
                  if((outputChConfig[idx] & 0x03) == SIGNAL_TYPE_SERVOPWM)
                  {
                    uint8_t servoPWMRangeIdx = (outputChConfig[idx] >> 4) & 0x0F;
                    display.print(500 + (servoPWMRangeIdx * 50));
                    display.print(F(" to "));
                    display.print(2500 - (servoPWMRangeIdx * 50));
                    display.setCursor(display.getCursorX() + 3, ypos);
                    display.print(F("\xE6s"));
                  }
                  else
                    display.print(F("N/A"));
                }
              }
              
              //Draw scroll bar
              drawScrollBar(127, 9, numItems, topItem, 4, 44);
              
              //show the write button
              drawDottedHLine(0, 54, 128, BLACK, WHITE);
              display.setCursor(84, 56);
              display.print(F("[Write]"));
              if(focusedItem == numItems + 1)
                drawCursor(76, 56);
              
              //Handle navigation
              changeFocusOnUpDown(numItems + 1); //+1 for button focus
              if(focusedItem < topItem)
                topItem = focusedItem;
              while(focusedItem >= topItem + 4 && focusedItem < numItems + 1)
                topItem++;
              toggleEditModeOnSelectClicked();
            
              //edit parameters
              uint8_t idx = startIdx + focusedItem - 1;
              if(idx <= endIdx && ((outputChConfig[idx] & 0x03) == SIGNAL_TYPE_SERVOPWM))
              {
                uint8_t servoPWMRangeIdx = (outputChConfig[idx] >> 4) & 0x0F;
                servoPWMRangeIdx = incDec(servoPWMRangeIdx, 0, 15, INCDEC_NOWRAP, INCDEC_SLOW);
                outputChConfig[idx] &= 0x0F;
                outputChConfig[idx] |= servoPWMRangeIdx << 4;
              }
              
              //write configuration
              if(focusedItem == numItems + 1 && clickedButton == KEY_SELECT)
              {
                state = SENDING_CONFIG;
                entryTime = millis();
                gotOutputChConfig = false;
                actionStarted = false;
                viewInitialised = false;
              }
              
              //exit without writing changes
              if(heldButton == KEY_SELECT)
              {
                stateInitialised = false;
                actionStarted = false;
                viewInitialised = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
            
          case SENDING_CONFIG:
            {
              printFullScreenMessage(PSTR("Writing settings"));
              if(Sys.animationsEnabled)
                drawLoaderSpinner(60, display.getCursorY() + 12, 2);
              
              if(!actionStarted)
              {
                isSendOutputChConfig = true;
                actionStarted = true;
              }
              if(receiverConfigStatusCode > 0)
              {
                if(receiverConfigStatusCode == 1) makeToast(PSTR("Write success"), 2000, 0);
                if(receiverConfigStatusCode == 2) makeToast(PSTR("Write failed"), 2000, 0);
                stateInitialised = false;
                actionStarted = false;
                changeToScreen(SCREEN_RECEIVER);
              }
              //Time out
              if(millis() - entryTime > 3000)
              {
                makeToast(PSTR("No response"), 2000, 0);
                stateInitialised = false;
                actionStarted = false;
                changeToScreen(SCREEN_RECEIVER);
              }
            }
            break;
        }
      }
      break;
  }
}

#endif
