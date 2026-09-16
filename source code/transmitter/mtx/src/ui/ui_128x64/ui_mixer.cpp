#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_mixer()
{
  switch(theScreen)
  {
    case SCREEN_MIXER:
      {
        drawHeader(mainMenu[MAIN_MENU_MIXER]);
        
        mixer_params_t *mxr = &Model.Mixer[thisMixIdx];
        
        //For safety and to prevent unintended effects, we do not directly edit the mixer output variable 
        //but instead use some kind of delayed action, where we edit a temporary variable 
        //and later assign it to the output in the right conditions.
        static uint8_t tempMixerOutput;
        static bool tempInitialised = false;
        if(!tempInitialised)
        {
          tempMixerOutput = mxr->output;
          tempInitialised = true;
        }
        
        display.setCursor(8, 9);
        display.print(F("Mix"));
        display.print(thisMixIdx + 1);
        if(!isEmptyStr(mxr->name, sizeof(mxr->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(mxr->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

        //--- Dynamic list ---
        
        enum {
          ITEM_MIX_OUTPUT,
          ITEM_MIX_SWITCH,
          ITEM_MIX_OPERATION,
          ITEM_MIX_INPUT,
          ITEM_MIX_WEIGHT,
          ITEM_MIX_OFFSET,
          ITEM_MIX_CURVE_TYPE,
          ITEM_MIX_CURVE_VAL,
          ITEM_MIX_TRIM_ENABLED,
          ITEM_MIX_FLIGHT_MODE,
          ITEM_MIX_DELAY_UP,
          ITEM_MIX_DELAY_DOWN,
          ITEM_MIX_SLOW_UP,
          ITEM_MIX_SLOW_DOWN,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; //stores the added item IDs
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(mxr->operation == MIX_HOLD)
          {
            if(i != ITEM_MIX_OUTPUT && i != ITEM_MIX_SWITCH && i!= ITEM_MIX_OPERATION)
              continue;
          }
          if(i == ITEM_MIX_FLIGHT_MODE)
          {
            if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
              listItemIDs[listItemCount++] = i;
            continue;
          }
          listItemIDs[listItemCount++] = i;
        }
        
        //initialise the view
        static uint8_t topItem;
        static uint8_t lastFocusedItem;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = 1;
          lastFocusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }

        bool hasCrossHairs = true;
        
        //handle navigation
        focusedItem = lastFocusedItem;
        uint8_t numFocusable = listItemCount + 2; //+1 for title focus, +1 for context menu focus
        changeFocusOnUpDown(numFocusable); 
        toggleEditModeOnSelectClicked();
        if(focusedItem == 1 || focusedItem == numFocusable) //title focus or context menu focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        lastFocusedItem = focusedItem;
        
        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(48, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
    
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2]);
          bool edit = (isFocused && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_MIX_OUTPUT:
              {
                display.print(F("Output:"));
                display.setCursor(56, ypos);
                getSrcName(textBuff, tempMixerOutput, sizeof(textBuff));
                display.print(textBuff);
                if(tempMixerOutput >= SRC_CH1 && tempMixerOutput < SRC_CH1 + NUM_RC_CHANNELS)
                {
                  uint8_t chIdx = tempMixerOutput - SRC_CH1;
                  display.setCursor(display.getCursorX() + 6, ypos);
                  display.print(Model.Channel[chIdx].name);
                }
                if(edit && (buttonCode == KEY_UP || buttonCode == KEY_DOWN))
                {
                  do {
                  tempMixerOutput = incDecSource(tempMixerOutput, INCDEC_FLAG_MIX_SRC);
                  } while(tempMixerOutput > SRC_NONE && tempMixerOutput < SRC_CH1); //skip over
                }                
              }
              break;

            case ITEM_MIX_SWITCH:
              {
                display.print(F("Switch:"));
                display.setCursor(56, ypos);
                getControlSwitchName(textBuff, mxr->swtch, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                  mxr->swtch = incDecControlSwitch(mxr->swtch);
              }
              break;
            
            case ITEM_MIX_OPERATION:
              {
                display.print(F("Opertn:"));
                display.setCursor(56, ypos);
                display.print(findStringInIdStr(enum_MixerOperation, mxr->operation));
                if(edit)
                  mxr->operation = incDec(mxr->operation, 0, MIX_OPERATOR_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_MIX_INPUT:
              {
                display.print(F("Input:"));
                display.setCursor(56, ypos);
                getSrcName(textBuff, mxr->input, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                {
                  //auto detect moved source
                  uint8_t movedSrc = procMovedSource(getMovedSource());
                  if(movedSrc != SRC_NONE)
                    mxr->input = movedSrc;
                  //inc dec
                  mxr->input = incDecSource(mxr->input, INCDEC_FLAG_MIX_SRC | INCDEC_FLAG_COUNTER_AS_SRC);
                  //hide cross hairs on the graph to minimise distraction when editing the input
                  if(buttonCode == KEY_UP || buttonCode == KEY_DOWN)
                    hasCrossHairs = false;
                }
              }
              break;
              
            case ITEM_MIX_WEIGHT:
              {
                display.print(F("Weight:"));
                display.setCursor(56, ypos);
                display.print(mxr->weight);
                if(edit)
                  mxr->weight = incDec(mxr->weight, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_OFFSET:
              {
                display.print(F("Offset:"));
                display.setCursor(56, ypos);
                display.print(mxr->offset);
                if(edit)
                  mxr->offset = incDec(mxr->offset, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_CURVE_TYPE:
              {
                display.print(F("Curve:"));
                display.setCursor(56, ypos);
                display.print(findStringInIdStr(enum_MixerCurveType, mxr->curveType));
                if(edit)
                {
                  uint8_t prevCurveType = mxr->curveType;
                  mxr->curveType = incDec(mxr->curveType, 0, MIX_CURVE_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  if(mxr->curveType != prevCurveType) 
                    mxr->curveVal = 0; //reset value if changed
                }
              }
              break;
              
            case ITEM_MIX_CURVE_VAL:
              {
                display.print(F("Crv val:"));
                display.setCursor(56, ypos);
                if(mxr->curveType == MIX_CURVE_TYPE_DIFF || mxr->curveType == MIX_CURVE_TYPE_EXPO)
                  display.print(mxr->curveVal);
                else if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION)
                  display.print(findStringInIdStr(enum_MixerCurveType_Func, mxr->curveVal));
                else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
                {
                  if(isEmptyStr(Model.CustomCurve[mxr->curveVal].name, sizeof(Model.CustomCurve[0].name)))
                  {
                    display.print(F("Crv"));
                    display.print(mxr->curveVal + 1);
                  }
                  else
                    display.print(Model.CustomCurve[mxr->curveVal].name);
                }
                if(edit)
                {
                  if(mxr->curveType == MIX_CURVE_TYPE_DIFF || mxr->curveType == MIX_CURVE_TYPE_EXPO)
                    mxr->curveVal = incDec(mxr->curveVal, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                  else if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION)
                    mxr->curveVal = incDec(mxr->curveVal, 0, MIX_CURVE_FUNC_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
                    mxr->curveVal = incDec(mxr->curveVal, 0, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
              }
              break;
              
            case ITEM_MIX_TRIM_ENABLED:
              {
                display.print(F("Trim:"));
                display.setCursor(56, ypos);
                uint8_t input = mxr->input;
                if(input == SRC_RUD) input = Model.rudSrcRaw;
                if(input == SRC_THR) input = Model.thrSrcRaw;
                if(input == SRC_AIL) input = Model.ailSrcRaw;
                if(input == SRC_ELE) input = Model.eleSrcRaw;
                if(input == SRC_X1_AXIS || input == SRC_Y1_AXIS || input == SRC_X2_AXIS || input == SRC_Y2_AXIS)
                {
                  drawCheckbox(56, ypos, mxr->trimEnabled);
                  if(edit)
                    mxr->trimEnabled = incDec(mxr->trimEnabled, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
                else
                  display.print(F("N/A"));
              }
              break;
              
            case ITEM_MIX_FLIGHT_MODE:
              {
                display.print(F("F-mode:"));
                display.setCursor(56, ypos);
                if(mxr->flightMode == 0xff)
                  display.print(F("All"));
                else
                {
                  bool delimit = false;
                  for(uint8_t i = 0; i < NUM_FLIGHT_MODES; i++)
                  {
                    if((mxr->flightMode >> i) & 0x01)
                    {
                      if(delimit)
                        display.print(F(","));
                      display.print(i+1);
                      delimit = true;
                    }
                  }
                }
                if(edit)
                  changeToScreen(DIALOG_MIX_FLIGHT_MODE);
              }
              break;
              
            case ITEM_MIX_DELAY_UP:
              {
                display.print(F("Dly up:"));
                display.setCursor(56, ypos);
                printSeconds(mxr->delayUp);
                if(edit)
                  mxr->delayUp = incDec(mxr->delayUp, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_DELAY_DOWN:
              {
                display.print(F("Dly dn:"));
                display.setCursor(56, ypos);
                printSeconds(mxr->delayDown);
                if(edit)
                  mxr->delayDown = incDec(mxr->delayDown, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_SLOW_UP:
              {
                display.print(F("Slow up:"));
                display.setCursor(56, ypos);
                printSeconds(mxr->slowUp);
                if(edit)
                  mxr->slowUp = incDec(mxr->slowUp, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
              
            case ITEM_MIX_SLOW_DOWN:
              {
                display.print(F("Slow dn:"));
                display.setCursor(56, ypos);
                printSeconds(mxr->slowDown);
                if(edit)
                  mxr->slowDown = incDec(mxr->slowDown, 0, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //Draw mixer curve preview
        focusedMixIdx = thisMixIdx;
        if(Sys.showCurvePreviewInMixer && mxr->operation != MIX_HOLD)
          drawMixerCurvePreview(mxr, hasCrossHairs);
        
        //Show context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == numFocusable ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);
        
        //Assign from temp
        if(!isEditMode || (buttonCode == 0 && millis() - buttonReleaseTime > 1000))
        {
          mxr->output = tempMixerOutput;
          tempInitialised = false;
        }
        
        //change to next mix
        if(focusedItem == 1)
        {
          drawCursor(0, 9);
          if(isEditMode)
          {
            //assign from temp
            mxr->output = tempMixerOutput;
            tempInitialised = false;
            //change to another mix
            uint8_t prevMixIdx = thisMixIdx;
            thisMixIdx = incDec(thisMixIdx, 0, NUM_MIX_SLOTS - 1, INCDEC_WRAP, INCDEC_SLOW);
            if(thisMixIdx != prevMixIdx)
              graphYCoordinatesInvalid = true;
          }
        }
        
        //Open context menu
        if(focusedItem == numFocusable && clickedButton == KEY_SELECT)
        {
          changeToScreen(CONTEXT_MENU_MIXER);
          viewInitialised = false;
          //assign from temp
          mxr->output = tempMixerOutput;
          tempInitialised = false;
        }

        //Exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_MAIN_MENU);
          //assign from temp
          mxr->output = tempMixerOutput; 
          tempInitialised = false;
        } 
      }
      break;
      
    case DIALOG_MIX_FLIGHT_MODE:
      {
        //--- scrollable list

        //handle navigation
        changeFocusOnUpDown(NUM_FLIGHT_MODES);
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 4)
          topItem++;
        
        //Calculate y coordinate for item 0. Items are center aligned
        uint8_t numVisible = NUM_FLIGHT_MODES <= 4 ? NUM_FLIGHT_MODES : 4;
        uint8_t y0 = ((display.height() - (numVisible * 9)) / 2) + 1;  //9 is line height
        
        //draw bounding box
        drawBoundingBox(11, y0 - 3, 105, numVisible * 9 + 4, BLACK);
        
        //fill list
        for(uint8_t line = 0; line < numVisible; line++)
        {
          uint8_t ypos = y0 + line * 9;
          uint8_t item = topItem + line;
          if(focusedItem == item)
            drawCursor(17, ypos);
          uint8_t idx = item - 1;
          drawCheckbox(25, ypos, Model.Mixer[thisMixIdx].flightMode & (1 << idx));
          display.setCursor(35, ypos);
          getControlSwitchName(textBuff, CTRL_SW_FMD_FIRST + idx, sizeof(textBuff));
          display.print(textBuff);
          display.print(F(" "));
          display.print(Model.FlightMode[idx].name);
        }
        
        //scroll bar
        uint8_t  y = Sys.useRoundRect ? y0 : y0 - 1;
        uint16_t h = Sys.useRoundRect ? numVisible * 9 - 2 : numVisible * 9;
        drawScrollBar(112, y, NUM_FLIGHT_MODES, topItem, numVisible, h);
        
        //--- edit items
        toggleEditModeOnSelectClicked();
        if(isEditMode && (pressedButton == KEY_UP || pressedButton == KEY_DOWN))
        {
          uint8_t idx = focusedItem - 1;
          Model.Mixer[thisMixIdx].flightMode ^= 1 << idx; //toggle bit
        }
        
        //--- exit 
        if(heldButton == KEY_SELECT)
        {
          if((Model.Mixer[thisMixIdx].flightMode & (uint8_t)((1 << NUM_FLIGHT_MODES) - 1)) == 0) //nothing selected
          {
            killButtonEvents();
            makeToast(PSTR("Select at least one"), 2000, 0);
          }
          else 
            changeToScreen(SCREEN_MIXER);
        }
      }
      break;

    case CONTEXT_MENU_MIXER:
      {
        enum {
          ITEM_MIXER_OUTPUTS,
          ITEM_COPY_MIX,
          ITEM_MOVE_MIX,
          ITEM_INSERT_FREE,
          ITEM_RENAME_MIX,
          ITEM_RESET_MIX,
          ITEM_RESET_ALL_MIXES,
          ITEM_COMPACT_MIXES,
          ITEM_MIXER_TEMPLATES,
          ITEM_MIXER_OVERVIEW,
          ITEM_TOGGLE_CURVE_PREVIEW,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_MIXER_OUTPUTS);
        contextMenuAddItem(PSTR("Copy mix to"), ITEM_COPY_MIX);
        contextMenuAddItem(PSTR("Move mix to"), ITEM_MOVE_MIX);
        contextMenuAddItem(PSTR("Insert free"), ITEM_INSERT_FREE);
        contextMenuAddItem(PSTR("Rename mix" ), ITEM_RENAME_MIX);
        contextMenuAddItem(PSTR("Reset mix" ), ITEM_RESET_MIX);
        contextMenuAddItem(PSTR("Reset all mixes"), ITEM_RESET_ALL_MIXES);
        contextMenuAddItem(PSTR("Compact mixes"), ITEM_COMPACT_MIXES);
        if(Sys.showCurvePreviewInMixer)
          contextMenuAddItem(PSTR("Hide curve preview"), ITEM_TOGGLE_CURVE_PREVIEW);
        else
          contextMenuAddItem(PSTR("Show curve preview"), ITEM_TOGGLE_CURVE_PREVIEW);
        if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
        {
          if(Sys.mixerTemplatesEnabled)
            contextMenuAddItem(PSTR("Templates"), ITEM_MIXER_TEMPLATES);
        }
        contextMenuAddItem(PSTR("Show overview"), ITEM_MIXER_OVERVIEW);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_MIXER_OUTPUTS) 
          changeToScreen(SCREEN_MIXER_OUTPUT);
        if(contextMenuSelectedItemID == ITEM_COPY_MIX)
        {
          destMixIdx = thisMixIdx;
          changeToScreen(DIALOG_COPY_MIX);
        }
        if(contextMenuSelectedItemID == ITEM_MOVE_MIX)
        {
          destMixIdx = thisMixIdx;
          changeToScreen(DIALOG_MOVE_MIX);
        }
        if(contextMenuSelectedItemID == ITEM_INSERT_FREE)
        {
          //Find a slot that is empty.
          //If the slot exists after thisMixIdx, move it here, otherwise we don't.
          uint8_t freeSlot = 0xFF;
          uint8_t mixIdx = NUM_MIX_SLOTS - 1;
          uint8_t occupiedCount = 0;
          while(mixIdx > thisMixIdx)
          {
            if(freeSlot == 0xFF && Model.Mixer[mixIdx].output == SRC_NONE && Model.Mixer[mixIdx].input == SRC_NONE)
              freeSlot = mixIdx;
            //count how many occupied slots exist between the free slot and the destination
            if(freeSlot != 0xFF && (Model.Mixer[mixIdx].output != SRC_NONE || Model.Mixer[mixIdx].input != SRC_NONE))
              occupiedCount++;
            mixIdx--;
          }
          if(freeSlot == 0xFF || (occupiedCount == 0 && Model.Mixer[thisMixIdx].output == SRC_NONE 
                                  && Model.Mixer[thisMixIdx].input == SRC_NONE))
          {
            makeToast(PSTR("Insertion failed"), 2000, 0);
          }
          else
          {
            resetMixerParams(freeSlot);
            moveMix(thisMixIdx, freeSlot);
          }
          changeToScreen(SCREEN_MIXER);
        }
        if(contextMenuSelectedItemID == ITEM_RENAME_MIX)
          changeToScreen(DIALOG_RENAME_MIX);
        if(contextMenuSelectedItemID == ITEM_RESET_MIX)
        {
          resetMixerParams(thisMixIdx);
          changeToScreen(SCREEN_MIXER);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_ALL_MIXES)
          changeToScreen(CONFIRMATION_MIXES_RESET);
        if(contextMenuSelectedItemID == ITEM_COMPACT_MIXES)
        {
          //Find all empty slots and move them to the end.
          //This is similar to the classic 'move all zeros to end of array' problem.
          
          //find the rearmost occupied slot
          uint8_t rearmostIdx = 0;
          for(uint8_t mixIdx = NUM_MIX_SLOTS - 1; mixIdx > 0; mixIdx--)
          {
            if(Model.Mixer[mixIdx].output != SRC_NONE || Model.Mixer[mixIdx].input != SRC_NONE)
            {
              rearmostIdx = mixIdx;
              break;
            }
          }
          //find and move all free slots to end
          uint8_t mixIdx = 0;
          while(mixIdx < rearmostIdx)
          {
            if(Model.Mixer[mixIdx].output == SRC_NONE && Model.Mixer[mixIdx].input == SRC_NONE)
            {
              moveMix(NUM_MIX_SLOTS - 1, mixIdx);
              rearmostIdx--;
              //we don't immediately increment the mixIdx to also move any adjacent free slots
            }
            else
              mixIdx++;
          }
          thisMixIdx = 0;
          changeToScreen(SCREEN_MIXER);
        }
        if(contextMenuSelectedItemID == ITEM_MIXER_TEMPLATES)
          changeToScreen(CONTEXT_MENU_MIXER_TEMPLATES);
        if(contextMenuSelectedItemID == ITEM_MIXER_OVERVIEW)
          changeToScreen(SCREEN_MIXER_OVERVIEW);
        if(contextMenuSelectedItemID == ITEM_TOGGLE_CURVE_PREVIEW)
        {
          Sys.showCurvePreviewInMixer = !Sys.showCurvePreviewInMixer;
          changeToScreen(SCREEN_MIXER);
        }
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case CONTEXT_MENU_MIXER_TEMPLATES:
    case CONFIRMATION_LOAD_MIXER_TEMPLATE:
      {
        enum {
          ITEM_BASIC,
          ITEM_ELEVON,
          ITEM_VTAIL,
          ITEM_DIFF_THRUST
        };
        
        if(theScreen == CONTEXT_MENU_MIXER_TEMPLATES)
        {
          contextMenuInitialise();
          contextMenuAddItem(PSTR("Basic"), ITEM_BASIC);
          if(Model.type == MODEL_TYPE_AIRPLANE)
          {
            contextMenuAddItem(PSTR("Elevon"), ITEM_ELEVON);
            contextMenuAddItem(PSTR("V-tail"), ITEM_VTAIL);
            contextMenuAddItem(PSTR("Differential thrust"), ITEM_DIFF_THRUST);
          }
          contextMenuDraw();
          
          if(contextMenuSelectedItemID != 0xff) //selection
            theScreen = CONFIRMATION_LOAD_MIXER_TEMPLATE;

          if(heldButton == KEY_SELECT) //exit
            changeToScreen(SCREEN_MIXER);
        }
        else if(theScreen == CONFIRMATION_LOAD_MIXER_TEMPLATE)
        {
          bool hasWarning = false;
          //basic
          if(contextMenuSelectedItemID == ITEM_BASIC) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateBasic(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          //elevon
          if(contextMenuSelectedItemID == ITEM_ELEVON) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateElevon(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          //vtail
          if(contextMenuSelectedItemID == ITEM_VTAIL) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateVtail(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          //differential thrust
          if(contextMenuSelectedItemID == ITEM_DIFF_THRUST) 
          {
            if(hasEnoughMixSlots(thisMixIdx, 4))
            {
              if(hasOccupiedMixSlots(thisMixIdx, 4))
                hasWarning = true;
              if(clickedButton == KEY_UP || !hasWarning)
              {
                loadMixerTemplateDiffThrust(thisMixIdx);
                changeToScreen(SCREEN_MIXER);
              }
            }
            else
              changeToScreen(SCREEN_MIXER);
          }
          
          //show warning
          if(hasWarning)
            printFullScreenMessage(PSTR("One or more\ndestination slots\nalready occupied.\nContinue?\n\nYes [Up] \nNo [Down]"));
          
          //exit
          if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
            changeToScreen(SCREEN_MIXER);
        }
      }
      break;

    case DIALOG_RENAME_MIX:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Mix name"), Model.Mixer[thisMixIdx].name, sizeof(Model.Mixer[0].name), true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case CONFIRMATION_MIXES_RESET:
      {
        printFullScreenMessage(PSTR("Reset all mixes?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          resetMixerParams();
          thisMixIdx = 0;
          destMixIdx = 0;
          changeToScreen(SCREEN_MIXER);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case SCREEN_MIXER_OUTPUT:
      {
        display.setInterlace(false); 
        
        drawHeader(PSTR("Mixer output"));
        
        uint8_t numPagesPrpCh = (NUM_RC_CHANNELS + 7) / 8;
        uint8_t numPagesVrtCh = (NUM_VIRTUAL_CHANNELS + 7) / 8;
        uint8_t numPages = numPagesPrpCh + numPagesVrtCh;
        static uint8_t thisPage = 1;
        static bool viewInitialised = false;
        static bool hasTooltip = false;
        static uint8_t tooltipSrcIdx = SRC_CH1;
        if(!viewInitialised) //start in the page that has the channel we want to view
        {
          uint8_t _outputIdx = Model.Mixer[thisMixIdx].output;
          if(_outputIdx >= SRC_CH1 && _outputIdx < (SRC_CH1 + NUM_RC_CHANNELS))
          {
            thisPage = ((_outputIdx - SRC_CH1) + 8) / 8;
            tooltipSrcIdx = _outputIdx;
          }
          else if(_outputIdx >= SRC_VIRTUAL_FIRST && _outputIdx <= SRC_VIRTUAL_LAST)
          {
            thisPage = numPagesPrpCh + ((_outputIdx - SRC_VIRTUAL_FIRST) + 8) / 8;
            tooltipSrcIdx = _outputIdx;
          }
          viewInitialised = true;
          hasTooltip = false;
        }
        
        //--- draw graphs---
        uint8_t startIdx = (thisPage - 1) * 8;
        uint8_t endIdx = NUM_RC_CHANNELS - 1;
        uint8_t offsetIdx = 0;
        display.setCursor(0, 7);
        if(thisPage <= numPagesPrpCh) //show real channels 
          display.print(F("Ch"));
        else //show virtual channels
        {  
          display.print(F("Virt"));
          startIdx = NUM_RC_CHANNELS + (8 * (thisPage - (numPagesPrpCh + 1)));
          endIdx = NUM_RC_CHANNELS + NUM_VIRTUAL_CHANNELS - 1;
          offsetIdx = NUM_RC_CHANNELS;
        }
        uint8_t count = 0;
        for(uint8_t i = startIdx; i < startIdx + 8 && i <= endIdx; i++)
        {
          int8_t outVal = mixSources[SRC_CH1 + i] / 25;
          uint8_t xOffset = (i - startIdx) * 16;
          if(outVal > 0)
            display.fillRect(6 + xOffset, 36 - outVal, 3, outVal , BLACK);
          else if(outVal < 0)
          {
            outVal = -outVal;
            display.fillRect(6 + xOffset, 35, 3, outVal, BLACK);
          }
          drawDottedVLine(7 + xOffset, 16, 39, BLACK, WHITE);
          //show the channel number
          uint8_t num = (i + 1) - offsetIdx;
          if(num < 10)
            display.setCursor(5 + xOffset, 57);
          else
            display.setCursor(2 + xOffset, 57);
          display.print(num);
          count++;
        }
        //draw midpoint
        display.drawHLine(3, 35, 9 + 16*(count - 1) , BLACK); 

        //show info tip
        if(clickedButton == KEY_SELECT)
          hasTooltip = !hasTooltip;
        if(hasTooltip)
        {
          //--- draw as an overlay
          itoa(mixSources[tooltipSrcIdx] / 5, textBuff, 10);
          uint8_t j = ((tooltipSrcIdx - SRC_CH1) + offsetIdx) % 8;
          drawTooltip(7 + j * 16, 14, textBuff);
          
          //--- scroll
          isEditMode = true;
          tooltipSrcIdx = incDec(tooltipSrcIdx, SRC_VIRTUAL_LAST, SRC_CH1, INCDEC_WRAP, INCDEC_SLOW);
          //set to the page that has our target
          if(tooltipSrcIdx >= SRC_CH1 && tooltipSrcIdx < (SRC_CH1 + NUM_RC_CHANNELS))
            thisPage = ((tooltipSrcIdx - SRC_CH1) + 8) / 8;
          else if(tooltipSrcIdx >= SRC_VIRTUAL_FIRST && tooltipSrcIdx <= SRC_VIRTUAL_LAST)
            thisPage = numPagesPrpCh + ((tooltipSrcIdx - SRC_VIRTUAL_FIRST) + 8) / 8;
        }

        //show scrollbar
        drawScrollBar(127, 8, numPages, thisPage, 1, 1 * 56);

        //change page
        if(!hasTooltip)
        {
          isEditMode = true;
          uint8_t prevPage = thisPage;
          thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
          if(thisPage != prevPage)
          {
            if(thisPage <= numPagesPrpCh)
              tooltipSrcIdx = SRC_CH1 + (thisPage - 1) * 8;
            else
              tooltipSrcIdx = SRC_CH1 + NUM_RC_CHANNELS + (8 * (thisPage - (numPagesPrpCh + 1)));
          }
        }

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_MIXER);
        }
      }
      break;
    
    case DIALOG_COPY_MIX:
    case DIALOG_MOVE_MIX:
      {
        isEditMode = true;
        destMixIdx = incDec(destMixIdx, 0, NUM_MIX_SLOTS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Mix"), thisMixIdx, destMixIdx, theScreen == DIALOG_COPY_MIX);
        if(clickedButton == KEY_SELECT)
        {
          if(theScreen == DIALOG_COPY_MIX)
            Model.Mixer[destMixIdx] = Model.Mixer[thisMixIdx];
          else
            moveMix(destMixIdx, thisMixIdx);
          thisMixIdx = destMixIdx; 
          changeToScreen(SCREEN_MIXER); 
        }
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MIXER);
      }
      break;
      
    case SCREEN_MIXER_OVERVIEW:
      {
        drawHeader(PSTR("Overview"));
        
        //--- dynamic scrollable list
        
        uint8_t listItemIDs[NUM_MIX_SLOTS];
        uint8_t listItemCount = 0;
        //add to list
        for(uint8_t i = 0; i < NUM_MIX_SLOTS; i++)
        {
          if(Model.Mixer[i].output == SRC_NONE)
            continue;
          if(Model.Mixer[i].input == SRC_NONE && Model.Mixer[i].operation != MIX_HOLD)
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        static uint8_t thisPage = 1;
        static bool hasTooltip = false;
        static uint8_t tooltipIdx = 0; //index in listItemIDs, also the focused index
        static uint8_t topIdx = 0;
        static bool viewInitialised = false;
        //start in the page that has the current mix index
        if(!viewInitialised)
        {
          bool pageFound = false;
          for(uint8_t i = 0; i < listItemCount; i++) //search in list
          {
            if(listItemIDs[i] == thisMixIdx)
            {
              pageFound = true;
              thisPage = (i / 6) + 1;
              tooltipIdx = i;
              topIdx = (thisPage - 1) * 6;
              break;
            }
          }
          if(!pageFound)
          {
            thisPage = 1;
            tooltipIdx = 0;
            topIdx = 0;
          }
          hasTooltip = false;
          viewInitialised = true;
        }

        if(listItemCount == 0)
          printFullScreenMessage(PSTR("No mixes defined yet"));
        else
        {
          uint8_t numPages = (listItemCount + 5) / 6;
          
          if(hasTooltip)
          {
            //scroll line by line
            isEditMode = true;
            tooltipIdx = incDec(tooltipIdx, listItemCount - 1, 0, INCDEC_WRAP, INCDEC_SLOW);
            if(tooltipIdx < topIdx)
              topIdx = tooltipIdx;
            while(tooltipIdx >= topIdx + 6)
              topIdx++;
          }
          else
          {
            //scroll page by page
            isEditMode = true;
            uint8_t prevPage = thisPage;
            thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
            if(thisPage != prevPage) //page has changed, recalculate
            {
              tooltipIdx = (thisPage - 1) * 6;
              topIdx = tooltipIdx;
            }
          }

          //draw the list
          for(uint8_t i = topIdx; i < topIdx + 6 && i < listItemCount; i++)
          {
            uint8_t ypos = 9 * (i - topIdx + 1);
            uint8_t mixIdx = listItemIDs[i];
            
            if(hasTooltip && i == tooltipIdx)
            {
              display.fillRect(0, ypos - 1, 126, 9, BLACK);
              display.setTextColor(WHITE);
            }

            //output
            display.setCursor(1, ypos);
            getSrcName(textBuff, Model.Mixer[mixIdx].output, sizeof(textBuff));
            display.print(textBuff);
            
            //operation
            display.setCursor(31, ypos);
            if(Model.Mixer[mixIdx].operation == MIX_ADD) display.print(F("+="));
            if(Model.Mixer[mixIdx].operation == MIX_MULTIPLY) display.print(F("*="));
            if(Model.Mixer[mixIdx].operation == MIX_REPLACE) display.print(F("="));
            if(Model.Mixer[mixIdx].operation == MIX_HOLD) display.print(F("Hold"));
            
            //weight and input
            if(Model.Mixer[mixIdx].operation != MIX_HOLD)
            {
              //weight, right aligned
              uint8_t digits = 1;
              uint8_t num = abs(Model.Mixer[mixIdx].weight);          
              while(num >= 10)
              {
                num /= 10;
                digits++;
              }
              uint8_t xpos = 61 - (digits - 1)*6;
              if(Model.Mixer[mixIdx].weight < 0)
                xpos -= 6;
              display.setCursor(xpos, ypos);
              display.print(Model.Mixer[mixIdx].weight);
              
              //input 
              display.setCursor(67, ypos);
              if(Model.Mixer[mixIdx].input >= SRC_COUNTER_FIRST && Model.Mixer[mixIdx].input <= SRC_COUNTER_LAST 
                 && Model.Mixer[mixIdx].swtch != CTRL_SW_NONE)
              {
                //fix for text overflowing when input is a counter and there is a switch assigned
                display.print(F("Cntr"));
                display.print(Model.Mixer[mixIdx].input - SRC_COUNTER_FIRST + 1);
              }
              else
              {
                getSrcName(textBuff, Model.Mixer[mixIdx].input, sizeof(textBuff));
                display.print(textBuff);
              }
            }
            
            //swtch, right aligned
            if(Model.Mixer[mixIdx].swtch != CTRL_SW_NONE)
            {
              getControlSwitchName(textBuff, Model.Mixer[mixIdx].swtch, sizeof(textBuff));
              uint8_t len = strlen(textBuff);
              uint8_t xpos = 127 - len * 6;
              display.fillRect(xpos, ypos, (len * 6) - 1, 8, display.getTextColor() == BLACK ? WHITE : BLACK); //clear area just in case the preceding text overflowed
              display.setCursor(xpos, ypos);
              display.print(textBuff);
            }
            
            display.setTextColor(BLACK); //restore
          }
          
          //scrollbar
          drawScrollBar(127, 8, listItemCount, topIdx + 1, 6, 6 * 9);

          //toggle tooltip
          if(clickedButton == KEY_SELECT && listItemCount != 0)
            hasTooltip = !hasTooltip;
          
          //draw tooltip as an overlay
          if(hasTooltip)
          {
            uint8_t mixIdx = listItemIDs[tooltipIdx];
            mixer_params_t *mxr = &Model.Mixer[mixIdx];
  
            char strQQ[3][11]; //3 lines, each 11 characters including null
            memset(strQQ, 0, sizeof(strQQ));
  
            // offset
            if(mxr->offset != 0)
            {
              strlcpy_P(strQQ[0], PSTR("Ofst: "), sizeof(strQQ[0]));
              itoa(mxr->offset, textBuff, 10);
              strlcat(strQQ[0], textBuff, sizeof(strQQ[0]));
            }
  
            // curves
            if(mxr->curveType == MIX_CURVE_TYPE_DIFF && mxr->curveVal != 0)
            {
              strlcpy_P(strQQ[1], PSTR("Diff: "), sizeof(strQQ[1]));
              itoa(mxr->curveVal, textBuff, 10);
              strlcat(strQQ[1], textBuff, sizeof(strQQ[1]));
            }
            else if(mxr->curveType == MIX_CURVE_TYPE_EXPO && mxr->curveVal != 0)
            {
              strlcpy_P(strQQ[1], PSTR("Expo: "), sizeof(strQQ[1]));
              itoa(mxr->curveVal, textBuff, 10);
              strlcat(strQQ[1], textBuff, sizeof(strQQ[1]));
            }
            else if(mxr->curveType == MIX_CURVE_TYPE_FUNCTION && mxr->curveVal != MIX_CURVE_FUNC_NONE)
            {
              strlcpy_P(strQQ[1], PSTR("Func: "), sizeof(strQQ[1]));
              strlcat(strQQ[1], findStringInIdStr(enum_MixerCurveType_Func, mxr->curveVal), sizeof(strQQ[1]));
            }
            else if(mxr->curveType == MIX_CURVE_TYPE_CUSTOM)
            {
              strlcpy_P(strQQ[1], PSTR("Crv"), sizeof(strQQ[1]));
              itoa(mxr->curveVal + 1, textBuff, 10);
              strlcat(strQQ[1], textBuff, sizeof(strQQ[1]));
            }
  
            //delay, slow
            bool hasDelay = (mxr->delayUp != 0 || mxr->delayDown != 0) ? true : false;
            bool hasSlow = (mxr->slowUp != 0 || mxr->slowDown != 0) ? true : false;
            if(hasDelay && hasSlow)
              strlcpy_P(strQQ[2], PSTR("Dly, Slow"), sizeof(strQQ[2]));
            else if(hasDelay)
              strlcpy_P(strQQ[2], PSTR("Dly"), sizeof(strQQ[2]));
            else if(hasSlow)
              strlcpy_P(strQQ[2], PSTR("Slow"), sizeof(strQQ[2]));
  
            //draw, skipping empty lines
            uint8_t lineCount = 0;
            for(uint8_t i = 0; i < 3; i++)
            {
              if(!isEmptyStr(strQQ[i], sizeof(strQQ[0])))
                lineCount++;
            }
            if(lineCount > 0)
            {
              bool isTopAligned = ((tooltipIdx - topIdx) >= 3) ? true : false;
              uint8_t ypos = isTopAligned ? (((tooltipIdx - topIdx) - 3) * 9 + 4 + ((3 - lineCount) * 8)) : ((tooltipIdx - topIdx) * 9 + 20);
              uint8_t height = lineCount * 8 + 3;
              uint8_t width = 65;
              //recalculate width if only Delay or Slow
              if(strlen(strQQ[0]) == 0 && strlen(strQQ[1]) == 0)
                width = strlen(strQQ[2]) * 6 + 5;
              //clear area
              display.fillRect(57, ypos - 2, width + 5, height + 6, WHITE); 
              //draw bounding frame
              if(Sys.useRoundRect)
                display.drawRoundRect(59, ypos, width, height, 1, BLACK); 
              else
                display.drawRect(59, ypos, width, height,BLACK); 
              //draw drop shadow
              if(Sys.useRoundRect)
              {
                display.drawHLine(61, ypos + height, width - 2, BLACK); 
                display.drawVLine(width + 59, ypos + 2, height - 2, BLACK);
                display.drawPixel(width + 58, ypos + height - 1, BLACK);
              }
              else
              {
                display.drawHLine(61, ypos + height, width - 1, BLACK); 
                display.drawVLine(width + 59, ypos + 2, height - 1, BLACK);
              }
              //draw connector
              if(isTopAligned)
                display.drawBitmap(66, ypos + height - 1, tooltip_connector_down_left, 5, 5, BLACK, WHITE); 
              else
                display.drawBitmap(66, ypos - 3, tooltip_connector_up_left, 4, 4, BLACK, WHITE);
              //print the strings
              uint8_t line = 0;
              for(uint8_t i = 0; i < 3; i++)
              {
                if(isEmptyStr(strQQ[i], sizeof(strQQ[0])))
                  continue;
                else
                {
                  display.setCursor(62, ypos + 2 + (line * 8));
                  display.print(strQQ[i]);
                }
                line++;
              }
            }
          }
        }
        
        //exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_MIXER);
        }
      }
      break;
  }
}

#endif
