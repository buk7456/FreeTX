#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_custom_notifications()
{
  switch(theScreen)
  {
    case SCREEN_NOTIFICATION_SETUP:
      {
        notification_params_t *notification = &Model.CustomNotification[thisNotificationIdx]; 
        
        if(isEditTextDialog) 
        {
          editTextDialog(PSTR("Text"), notification->text, sizeof(notification->text), true, true, false);
          break;
        }
        
        drawHeader(extrasMenu[EXTRAS_MENU_NOTIFICATION_SETUP]);
        
        display.setCursor(8, 9);
        display.print(F("Notification"));
        display.print(thisNotificationIdx + 1);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //use a temporary variable for the swtch and assign later 
        //to avoid annoying playback while scrolling through the options
        static uint8_t tempSwtch;
        static bool tempInitialised = false;
        if(!tempInitialised)
        {
          tempSwtch = notification->swtch;
          tempInitialised = true;
        }
        
        //--- dynamic scrollable list
        
        enum {
          ITEM_NOTIFICATION_ENABLED,
          ITEM_NOTIFICATION_SWITCH,
          ITEM_NOTIFICATION_TONE,
          ITEM_NOTIFICATION_TEXT,

          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          // if(!notification->enabled && i > ITEM_NOTIFICATION_ENABLED)
          //   continue;
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
            drawCursor(46, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2]);
          bool edit = (isFocused && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_NOTIFICATION_ENABLED:
              {
                display.print(F("Enable:"));
                drawCheckbox(54, ypos, notification->enabled);
                if(edit)
                  notification->enabled = incDec(notification->enabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_NOTIFICATION_SWITCH:
              {
                display.print(F("Switch:"));
                display.setCursor(54, ypos);
                getControlSwitchName(textBuff, tempSwtch, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  tempSwtch = incDecControlSwitch(tempSwtch);
              }
              break;
              
            case ITEM_NOTIFICATION_TONE:
              {
                display.print(F("Tone:"));
                display.setCursor(54, ypos);
                display.print(F("Tone"));
                display.print(notification->tone - AUDIO_NOTIFICATION_TONE_FIRST + 1);
                if(isFocused)
                {
                  //preview the tone while in edit mode
                  static bool editStarted = false;
                  static uint8_t prevTone;
                  if(isEditMode && !editStarted)
                  {
                    editStarted = true;
                    prevTone = notification->tone;
                  }
                  if(!isEditMode || heldButton == KEY_SELECT)
                  {
                    prevTone = notification->tone;
                    editStarted = false;
                  }
                  notification->tone = incDec(notification->tone, AUDIO_NOTIFICATION_TONE_FIRST, AUDIO_NOTIFICATION_TONE_LAST, INCDEC_WRAP, INCDEC_SLOW);
                  if(notification->tone != prevTone && buttonCode == 0 && millis() - buttonReleaseTime >= 500) 
                  {
                    prevTone = notification->tone;
                    audioToPlay = notification->tone;
                  }
                }
              }
              break;
              
            case ITEM_NOTIFICATION_TEXT:
              {
                display.print(F("Text:"));
                display.setCursor(54, ypos);
                if(isEmptyStr(notification->text, sizeof(notification->text)))
                  display.print(F("--"));
                else
                  display.print(notification->text);
                if(edit)
                  isEditTextDialog = true;
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
          changeToScreen(CONTEXT_MENU_NOTIFICATIONS);
        
        //change to next
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisNotificationIdx = incDec(thisNotificationIdx, 0, NUM_CUSTOM_NOTIFICATIONS - 1, INCDEC_WRAP, INCDEC_SLOW);
          //assign from temp
          notification->swtch = tempSwtch;
          tempInitialised = false;
        }

        //assign from temp
        if(!isEditMode || (buttonCode == 0 && millis() - buttonReleaseTime >= 500))
        {
          notification->swtch = tempSwtch;
          tempInitialised = false;
        }

        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_EXTRAS_MENU);
          //assign from temp
          notification->swtch = tempSwtch;
          tempInitialised = false;
        }
      }
      break;
      
    case CONTEXT_MENU_NOTIFICATIONS:
      {
        enum {
          ITEM_COPY_TO,
          ITEM_RESET_SETTINGS,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_TO);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_SETTINGS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_COPY_TO)
        {
          destNotificationIdx = thisNotificationIdx;
          changeToScreen(DIALOG_COPY_NOTIFICATION);
        }
        
        if(contextMenuSelectedItemID == ITEM_RESET_SETTINGS)
        {
          resetNotificationParams(thisNotificationIdx);
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
      }
      break;
      
    case DIALOG_COPY_NOTIFICATION:
      {
        isEditMode = true;
        destNotificationIdx = incDec(destNotificationIdx, 0, NUM_CUSTOM_NOTIFICATIONS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("#"), thisNotificationIdx, destNotificationIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.CustomNotification[destNotificationIdx] = Model.CustomNotification[thisNotificationIdx];
          thisNotificationIdx = destNotificationIdx;
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_NOTIFICATION_SETUP);
      }
      break;
  }
}

#endif

