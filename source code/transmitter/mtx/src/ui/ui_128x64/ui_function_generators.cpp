#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_function_generators()
{
  switch(theScreen)
  {
    case SCREEN_FUNCTION_GENERATORS:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_FUNCTION_GENERATORS]);
        
        funcgen_t *fgen = &Model.Funcgen[thisFgenIdx];
        
        if(fgen->waveform == FUNCGEN_WAVEFORM_PULSE)
        {
          if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_FIXED && fgen->width >= fgen->period)
          {
            if(fgen->width >= 600)
              fgen->width = 599;
            fgen->period = fgen->width + 1;
          }
        }
        else
        {
          if(fgen->periodMode == FUNCGEN_PERIODMODE_VARIABLE && fgen->period1 >= fgen->period2)
          {
            if(fgen->period1 >= 600)
              fgen->period1 = 599;
            fgen->period2 = fgen->period1 + 1;
          }
        }
        
        display.setCursor(8, 9);
        getSrcName(textBuff, SRC_FUNCGEN_FIRST + thisFgenIdx, sizeof(textBuff));
        display.print(textBuff);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //--- dynamic scrollable list 
        
        enum {
          ITEM_WAVEFORM,
          ITEM_PERIOD_MODE,
          ITEM_PERIOD1,
          ITEM_PERIOD2,
          ITEM_WIDTH_MODE,
          ITEM_MODULATOR_SRC,
          ITEM_REVERSE_MODULATOR,
          ITEM_WIDTH,
          ITEM_PERIOD, 
          ITEM_PHASE_MODE,
          ITEM_PHASE,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(fgen->waveform == FUNCGEN_WAVEFORM_PULSE)
          {
            if(i == ITEM_PERIOD_MODE || i == ITEM_PERIOD1 || i == ITEM_PERIOD2)
              continue;
            if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_FIXED)
            {
              if(i == ITEM_MODULATOR_SRC || i == ITEM_REVERSE_MODULATOR)
                continue;
            }
            if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_VARIABLE)
            {
              if(i == ITEM_WIDTH)
                continue;
            }
          }
          else
          {
            if(i == ITEM_WIDTH_MODE || i == ITEM_WIDTH || i == ITEM_PERIOD)
              continue;
            if(fgen->periodMode == FUNCGEN_PERIODMODE_FIXED)
            {
              if(i == ITEM_PERIOD2 || i == ITEM_MODULATOR_SRC || i == ITEM_REVERSE_MODULATOR)
                continue;
            }
            if(fgen->periodMode == FUNCGEN_PERIODMODE_VARIABLE /*|| fgen->waveform == FUNCGEN_WAVEFORM_RANDOM*/)
            {
              if(i == ITEM_PHASE_MODE || i == ITEM_PHASE)
                continue;
            }
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
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(70, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_WAVEFORM:
              {
                display.print(F("Waveform:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenWaveform, fgen->waveform));
                if(edit)
                {
                  fgen->waveform = incDec(fgen->waveform, 0, FUNCGEN_WAVEFORM_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
              }
              break;
              
            case ITEM_PERIOD_MODE:
              {
                if(fgen->waveform == FUNCGEN_WAVEFORM_RANDOM) 
                  display.print(F("Intrvl mode:"));
                else 
                  display.print(F("Period mode:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenPeriodMode, fgen->periodMode));
                if(edit)
                  fgen->periodMode = incDec(fgen->periodMode, 0, FUNCGEN_PERIODMODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;

            case ITEM_PERIOD1:
              {
                if(fgen->waveform == FUNCGEN_WAVEFORM_RANDOM)
                {
                  if(fgen->periodMode == FUNCGEN_PERIODMODE_FIXED)
                    display.print(F("Interval:"));
                  else
                    display.print(F("Min intrvl:"));
                }
                else
                {
                  if(fgen->periodMode == FUNCGEN_PERIODMODE_FIXED)
                    display.print(F("Period:"));
                  else
                    display.print(F("Min period:"));
                }
                display.setCursor(78, ypos);
                printSeconds(fgen->period1);
                if(edit)
                {
                  uint16_t max = 600;
                  if(fgen->periodMode == FUNCGEN_PERIODMODE_VARIABLE)
                    max = fgen->period2 - 1;
                  fgen->period1 = incDec(fgen->period1, 1, max, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
              
            case ITEM_PERIOD2:
              {
                if(fgen->waveform == FUNCGEN_WAVEFORM_RANDOM)
                  display.print(F("Max intrvl:"));
                else
                  display.print(F("Max period:"));
                display.setCursor(78, ypos);
                printSeconds(fgen->period2);
                if(edit)
                  fgen->period2 = incDec(fgen->period2, fgen->period1 + 1, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;

            case ITEM_WIDTH_MODE:
              {
                display.print(F("Width mode:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenWidthMode, fgen->widthMode));
                if(edit)
                  fgen->widthMode = incDec(fgen->widthMode, 0, FUNCGEN_PULSE_WIDTH_MODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_WIDTH:
              {
                display.print(F("Width:"));
                display.setCursor(78, ypos);
                printSeconds(fgen->width);
                if(edit)
                  fgen->width = incDec(fgen->width, 1, fgen->period - 1, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
              }
              break;
            
            case ITEM_PERIOD:
              {
                display.print(F("Period:"));
                display.setCursor(78, ypos);
                printSeconds(fgen->period);
                if(edit)
                {
                  uint16_t min = 1;
                  if(fgen->widthMode == FUNCGEN_PULSE_WIDTH_FIXED)
                    min = fgen->width + 1;
                  fgen->period = incDec(fgen->period, min, 600, INCDEC_NOWRAP, INCDEC_SLOW, INCDEC_NORMAL);
                }
              }
              break;
            
            case ITEM_MODULATOR_SRC:
              {
                display.print(F("Modulator:"));
                display.setCursor(78, ypos);
                getSrcName(textBuff, fgen->modulatorSrc, sizeof(textBuff));
                display.print(textBuff);
                if(edit)
                {
                  uint8_t movedSrc = procMovedSource(getMovedSource());
                  if(movedSrc != SRC_NONE)
                    fgen->modulatorSrc = movedSrc;
                  //inc dec
                  fgen->modulatorSrc = incDecSource(fgen->modulatorSrc, INCDEC_FLAG_MIX_SRC);
                }
              }
              break;
              
            case ITEM_REVERSE_MODULATOR:
              {
                display.print(F("Reverse:"));
                drawCheckbox(78, ypos, fgen->reverseModulator);
                if(edit)
                  fgen->reverseModulator = incDec(fgen->reverseModulator, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_PHASE_MODE:
              {
                display.print(F("Phase mode:"));
                display.setCursor(78, ypos);
                display.print(findStringInIdStr(enum_FuncgenPhaseMode, fgen->phaseMode));
                if(edit)
                  fgen->phaseMode = incDec(fgen->phaseMode, 0, FUNCGEN_PHASEMODE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
              
            case ITEM_PHASE:
              {
                display.print(F("Phase:"));
                display.setCursor(78, ypos);
                if(fgen->phaseMode == FUNCGEN_PHASEMODE_AUTO)
                  display.print(F("[Sync]"));
                else if(fgen->phaseMode == FUNCGEN_PHASEMODE_FIXED)
                {
                  display.print(fgen->phase);
                  display.write(248); //degree symbol
                }
                
                if(edit)
                {
                  if(fgen->phaseMode == FUNCGEN_PHASEMODE_AUTO)
                  {
                    syncWaveform(thisFgenIdx);
                    makeToast(PSTR("Synchronized"), 2000, 0);
                    isEditMode = false;
                  }
                  else if(fgen->phaseMode == FUNCGEN_PHASEMODE_FIXED)
                    fgen->phase = incDec(fgen->phase, 0, 360, INCDEC_NOWRAP, INCDEC_NORMAL);
                }
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
          changeToScreen(CONTEXT_MENU_FUNCGEN);
        
        //change to next function generator
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisFgenIdx = incDec(thisFgenIdx, 0, NUM_FUNCGEN - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
      
    case CONTEXT_MENU_FUNCGEN:
      {
        enum {
          ITEM_VIEW_OUTPUTS,
          ITEM_COPY_FUNCGEN,
          ITEM_RESET_FUNCGEN,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("View outputs"), ITEM_VIEW_OUTPUTS);
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_FUNCGEN);
        contextMenuAddItem(PSTR("Reset settings"), ITEM_RESET_FUNCGEN);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RESET_FUNCGEN)
        {
          resetFuncgenParams(thisFgenIdx);
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
        }
        if(contextMenuSelectedItemID == ITEM_COPY_FUNCGEN)
        {
          destFgenIdx = thisFgenIdx;
          changeToScreen(DIALOG_COPY_FUNCGEN);
        }
        if(contextMenuSelectedItemID == ITEM_VIEW_OUTPUTS)
          changeToScreen(SCREEN_FUNCGEN_OUTPUTS);

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
      }
      break;
      
    case DIALOG_COPY_FUNCGEN:
      {
        isEditMode = true;
        destFgenIdx = incDec(destFgenIdx, 0, NUM_FUNCGEN - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Fgen"), thisFgenIdx, destFgenIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.Funcgen[destFgenIdx] = Model.Funcgen[thisFgenIdx];
          thisFgenIdx = destFgenIdx;
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
      }
      break;
      
    case SCREEN_FUNCGEN_OUTPUTS:
      {
        display.setInterlace(false); 
        drawHeader(PSTR("Funcgen outputs"));

        //--- scrollable 
        
        uint8_t numPages = (NUM_FUNCGEN + 4) / 5;
        static uint8_t thisPage = 1;
        
        static bool viewInitialised = false;
        if(!viewInitialised) 
        {
          thisPage = (thisFgenIdx + 5) / 5; //start in the page with the function generator we want to view
          viewInitialised = true;
        }

        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint8_t startIdx = (thisPage - 1) * 5;
        for(uint8_t i = startIdx; i < startIdx + 5 && i < NUM_FUNCGEN; i++)
        {
          uint8_t ypos = 10 + (i - startIdx) * 11;
          //show marker
          if(i == thisFgenIdx) 
          {
            display.setCursor(0, ypos);
            display.write(0xB1);
          }
          //show name
          display.setCursor(7, ypos);
          getSrcName(textBuff, SRC_FUNCGEN_FIRST + i, sizeof(textBuff));
          display.print(textBuff);
          display.print(F(":"));
          //draw graph
          drawHorizontalBarChartZeroCentered(45, ypos + 2, 39, 3, BLACK, mixSources[SRC_FUNCGEN_FIRST + i]/5, 200);
          //show value
          display.setCursor(94, ypos);
          display.print(mixSources[SRC_FUNCGEN_FIRST + i]/5);
        }
        
        //show scrollbar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        if(heldButton == KEY_SELECT) //exit
        {
          viewInitialised = false;
          changeToScreen(SCREEN_FUNCTION_GENERATORS);
        }
      }
      break;
  }
}

#endif
