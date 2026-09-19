#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_safety_checks()
{
  switch(theScreen)
  {
    case SCREEN_SAFETY_CHECKS:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_SAFETY_CHECKS]);
        
        //--- dynamic scrollable list
        enum {
          ITEM_SAFETY_CHECK_THROTTLE,
          ITEM_SAFETY_CHECK_SW_FIRST,
          ITEM_SAFETY_CHECK_SW_LAST = ITEM_SAFETY_CHECK_SW_FIRST + NUM_PHYSICAL_SWITCHES - 1,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {        
          if(i == ITEM_SAFETY_CHECK_THROTTLE && Model.type == MODEL_TYPE_OTHER)
            continue;
          //skip absent switches
          if(i >= ITEM_SAFETY_CHECK_SW_FIRST && i <= ITEM_SAFETY_CHECK_SW_LAST)
          {
            uint8_t sw = i - ITEM_SAFETY_CHECK_SW_FIRST;
            if(Sys.swType[sw] == SW_ABSENT)
              continue; 
          }
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount);
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        
        toggleEditModeOnSelectClicked();
        
        //fill list and edit items
        for(uint8_t line = 0; line < 6 && line < listItemCount; line++)
        {
          uint8_t ypos = 9 + line*9;
          if(focusedItem == topItem + line)
            drawCursor(64, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (itemID == listItemIDs[focusedItem - 1] && isEditMode);
          
          display.setCursor(0, ypos);
          
          if(itemID == ITEM_SAFETY_CHECK_THROTTLE)
          {
            display.print(F("Throttle:"));
            drawCheckbox(72, ypos, Model.checkThrottle);
            if(edit)
              Model.checkThrottle = incDec(Model.checkThrottle, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
          }
          else if(itemID >= ITEM_SAFETY_CHECK_SW_FIRST && itemID <= ITEM_SAFETY_CHECK_SW_LAST)
          {
            uint8_t sw = itemID - ITEM_SAFETY_CHECK_SW_FIRST;
            display.print(F("Switch "));
            display.write(65 + sw);
            display.print(F(":"));
            display.setCursor(72, ypos);
            display.print(findStringInIdStr(enum_SwitchWarn, Model.switchWarn[sw]));
            if(edit)
            {
              if(Sys.swType[sw] == SW_2POS || Sys.swType[sw] == SW_2POS_MOMENTARY)
                Model.switchWarn[sw] = incDec(Model.switchWarn[sw], -1, 1, INCDEC_WRAP, INCDEC_SLOW);
              else if(Sys.swType[sw] == SW_3POS || Sys.swType[sw] == SW_3POS_MOMENTARY)
                Model.switchWarn[sw] = incDec(Model.switchWarn[sw], -1, 2, INCDEC_WRAP, INCDEC_SLOW);
            }
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 9, listItemCount, topItem, 6, 6 * 9);

        // Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
  }
}

#endif
