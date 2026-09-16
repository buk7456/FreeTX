#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_inputs()
{
  switch(theScreen)
  {
    case SCREEN_INPUTS:
      {
        drawHeader(mainMenu[MAIN_MENU_INPUTS]);

        enum { 
          PAGE_AIL_CURVE, 
          PAGE_ELE_CURVE, 
          PAGE_RUD_CURVE, 
          PAGE_THR_CURVE,
          PAGE_STICKS,
          PAGE_KNOBS,
          PAGE_SWITCHES,
          
          NUM_PAGES
        };
        
        static uint8_t page;
        
        //check if the page is actually available to prevent getting stuck in an infinite loop
        if(Model.type == MODEL_TYPE_OTHER 
          && (page == PAGE_RUD_CURVE || page == PAGE_AIL_CURVE || page == PAGE_ELE_CURVE || page == PAGE_THR_CURVE))
        {
          page = PAGE_STICKS;
        }
        
        toggleEditModeOnSelectClicked();
        if(focusedItem == 1)
        {
          uint8_t lastPage = page;

          if(Model.type == MODEL_TYPE_AIRPLANE || Model.type == MODEL_TYPE_MULTICOPTER)
            page = incDec(page, 0, NUM_PAGES - 1, INCDEC_WRAP, INCDEC_SLOW);
          else if(Model.type == MODEL_TYPE_OTHER) 
          {
            //hide rud, ail, ele, thr curves
            do {
              page = incDec(page, 0, NUM_PAGES - 1, INCDEC_WRAP, INCDEC_SLOW);
            } while(page == PAGE_RUD_CURVE || page == PAGE_AIL_CURVE || page == PAGE_ELE_CURVE || page == PAGE_THR_CURVE);
          }

          if(page != lastPage)
            graphYCoordinatesInvalid = true;

          //Show cursor
          drawCursor(0, 9);
        }
          
        //------ RUD, AIL, ELE CURVES 
        if(page == PAGE_RUD_CURVE || page == PAGE_AIL_CURVE || page == PAGE_ELE_CURVE)  
        {  
          changeFocusOnUpDown(5);
          
          uint8_t qqPage[3] = {PAGE_RUD_CURVE, PAGE_AIL_CURVE, PAGE_ELE_CURVE};
          uint8_t qqSrc[3] = {SRC_RUD, SRC_AIL, SRC_ELE};
          
          rate_expo_t* qqRateExpo[3];
          qqRateExpo[0] = &Model.RudDualRate;
          qqRateExpo[1] = &Model.AilDualRate;
          qqRateExpo[2] = &Model.EleDualRate;
          
          uint8_t* qqSrcRaw[3];
          qqSrcRaw[0] = &Model.rudSrcRaw;
          qqSrcRaw[1] = &Model.ailSrcRaw;
          qqSrcRaw[2] = &Model.eleSrcRaw;
          
          int16_t qqValIn[3];
          qqValIn[0] = mixSources[Model.rudSrcRaw];
          qqValIn[1] = mixSources[Model.ailSrcRaw];
          qqValIn[2] = mixSources[Model.eleSrcRaw];
          
          uint8_t idx = 0;
          for(uint8_t i = 0; i < 3; i++) //find the idx
          {
            if(qqPage[i] == page)
            {
              idx = i;
              break;
            }
          }
          
          int8_t  *rate = &(qqRateExpo[idx]->rate1);
          int8_t  *expo = &(qqRateExpo[idx]->expo1);
          uint8_t *swtch = &(qqRateExpo[idx]->swtch);
          if(*swtch != CTRL_SW_NONE && checkSwitchCondition(*swtch))
          {
            rate = &(qqRateExpo[idx]->rate2);
            expo = &(qqRateExpo[idx]->expo2);
          }
          
          //--- Edit values
          if(focusedItem == 2 && isEditMode)
          {
            //auto detect moved
            uint8_t movedSrc = getMovedSource();
            if(movedSrc != SRC_NONE)
            {
              if(movedSrc >= SRC_RAW_ANALOG_FIRST && movedSrc <= SRC_RAW_ANALOG_LAST)
                *qqSrcRaw[idx] = movedSrc;
            }
            //inc dec
            *qqSrcRaw[idx] = incDecSource(*qqSrcRaw[idx], INCDEC_FLAG_MIX_SRC_RAW_ANALOG);
          }
          else if(focusedItem == 3)
            *rate = incDec(*rate, 0, 100, INCDEC_NOWRAP, INCDEC_NORMAL); 
          else if(focusedItem == 4)
            *expo = incDec(*expo, -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
          else if(focusedItem == 5 && isEditMode)
            *swtch = incDecControlSwitch(*swtch);
          
          //--- Draw text
          
          display.setCursor(8, 9);
          getSrcName(textBuff, qqSrc[idx], sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          display.setCursor(0, 20);
          display.print(F("Src:"));
          display.setCursor(42, 20);
          getSrcName(textBuff, *qqSrcRaw[idx], sizeof(textBuff));
          display.print(textBuff);

          display.setCursor(0, 29);
          display.print(F("Rate:"));
          display.setCursor(42, 29);
          display.print(*rate);
          display.print(F("%"));
          
          display.setCursor(0, 38);
          display.print(F("Expo:"));
          display.setCursor(42, 38);
          display.print(*expo);
          display.print(F("%"));
          
          display.setCursor(0, 47);
          display.print(F("D/R:"));
          display.setCursor(42, 47);
          getControlSwitchName(textBuff, *swtch, sizeof(textBuff));
          display.print(textBuff);
          
          //--- Show the current D/R in use. Only show if we have dual rates enabled
          if(*swtch != CTRL_SW_NONE)
          {
            display.setCursor(0, 56);
            if(checkSwitchCondition(*swtch))
              display.print(F("(Rate2)"));
            else
              display.print(F("(Rate1)"));
          }
          
          if(focusedItem > 1)
            drawCursor(34, (focusedItem * 9) + 2);

          //--- Draw graph
          
          //draw x y axis lines
          display.drawVLine(100, 11, 51, BLACK);
          display.drawHLine(75, 36, 51, BLACK);
          
          //draw stick input marker
          static int8_t lastVal = 0;
          static uint32_t lastMovedTime = 0;
          int16_t difference = qqValIn[idx]/5 - lastVal;
          if(difference >= 5 || difference <= -5)
          {
            lastVal = qqValIn[idx]/5;
            lastMovedTime = millis();
          }
          if(millis() - lastMovedTime < 3000)
          {
            display.setInterlace(false);
            drawDottedVLine(100 + qqValIn[idx]/20, 11, 51, BLACK, WHITE);
            int8_t y = calcRateExpo(qqValIn[idx], *rate, *expo) / 20;
            drawDottedHLine(75, 36 - y, 51, BLACK, WHITE);
          }
          
          //cache the y values so we don't have to recalculate them every time
          static int8_t lastRate, lastExpo;
          if(lastRate != *rate || lastExpo != *expo || graphYCoordinatesInvalid)
          {
            lastRate = *rate;
            lastExpo = *expo;
            graphYCoordinatesInvalid = false;
            for(int16_t i = 0; i <= 25; i++)
              mySharedUnion.graphYCoord[i] = calcRateExpo(i * 20, *rate, *expo) / 20;
          }
          
          //plot the points
          for(int16_t i = 0; i <= 25; i++)
          {
            display.drawPixel(100 + i, 36 - mySharedUnion.graphYCoord[i], BLACK);
            display.drawPixel(100 - i, 36 + mySharedUnion.graphYCoord[i], BLACK);
            if(i > 0)
            {
              int8_t y0 = mySharedUnion.graphYCoord[i - 1];
              int8_t y1 = mySharedUnion.graphYCoord[i];
              if(abs(y1 - y0) > 1)
              {
                display.drawLine(100 + i - 1, 36 - y0, 100 + i, 36 - y1, BLACK);
                display.drawLine(100 - i + 1, 36 + y0, 100 - i, 36 + y1, BLACK);
              }
            }
          }
        }
    
        //------ THROTTLE CURVE
        if(page == PAGE_THR_CURVE)
        {
          display.setCursor(8, 9);
          getSrcName(textBuff, SRC_THR, sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          custom_curve_t *crv = &Model.ThrottleCurve;
          
          static uint8_t thisPt = 0;
          if(thisPt >= crv->numPoints)
            thisPt = 0;
          
          //scrollable list
          
          enum {
            ITEM_THR_SRC_RAW,
            ITEM_CURVE_NUM_POINTS,
            ITEM_CURVE_POINT,
            ITEM_CURVE_XVAL,
            ITEM_CURVE_YVAL,
            ITEM_CURVE_SMOOTH,
            
            ITEM_COUNT
          };

          //handle navigation
          changeFocusOnUpDown(ITEM_COUNT + 1); //+1 for title focus
          static uint8_t topItem = 1;
          if(focusedItem == 1)
            topItem = 1;
          else if(focusedItem > 1)
          {
            if(focusedItem - 1 < topItem)
              topItem = focusedItem - 1;
            while(focusedItem - 1 >= topItem + 5)
              topItem++;
          }
          
          bool markNode = false;
          
          //fill list and edit items
          for(uint8_t line = 0; line < 5 && line < ITEM_COUNT; line++)
          {
            uint8_t ypos = 20 + line*9;
            if(focusedItem - 1 == topItem + line)
              drawCursor(30, ypos);
            
            if((topItem - 1 + line) >= ITEM_COUNT)
              break;
            
            uint8_t itemID = topItem - 1 + line;
            bool isFocused = (focusedItem > 1 && itemID == focusedItem - 2);
            bool edit = (isFocused && isEditMode);
            
            display.setCursor(0, ypos);
            switch(itemID)
            {
              case ITEM_THR_SRC_RAW:
                {
                  display.print(F("Src:"));
                  display.setCursor(38, ypos);
                  getSrcName(textBuff, Model.thrSrcRaw, sizeof(textBuff));
                  display.print(textBuff);
                  if(edit)
                  {
                    //auto detect moved
                    uint8_t movedSrc = getMovedSource();
                    if(movedSrc != SRC_NONE)
                    {
                      if(movedSrc >= SRC_RAW_ANALOG_FIRST && movedSrc <= SRC_RAW_ANALOG_LAST)
                        Model.thrSrcRaw = movedSrc;
                    }
                    //inc dec
                    Model.thrSrcRaw = incDecSource(Model.thrSrcRaw, INCDEC_FLAG_MIX_SRC_RAW_ANALOG);
                  }
                }
                break;
              
              case ITEM_CURVE_NUM_POINTS:
                {
                  display.print(F("Curv:"));
                  display.setCursor(38, ypos);
                  display.print(crv->numPoints);
                  display.setCursor(display.getCursorX() + 3, ypos);
                  display.print(F("pts"));
                  if(edit)
                  {
                    uint8_t prevNumPoints = crv->numPoints;
                    crv->numPoints = incDec(crv->numPoints, MIN_NUM_POINTS_CUSTOM_CURVE, 
                                                    MAX_NUM_POINTS_CUSTOM_CURVE, INCDEC_NOWRAP, INCDEC_SLOW);
                    if(crv->numPoints != prevNumPoints) //recalculate points if changed
                    {
                      calcNewCurvePts(crv, prevNumPoints);
                      thisPt = 0;
                    }
                  }
                }
                break;
              
              case ITEM_CURVE_POINT:
                {
                  display.print(F("Pt:"));
                  display.setCursor(38, ypos);
                  display.write(97 + thisPt);
                  if(isFocused)
                    markNode = true;
                  if(edit)
                    thisPt = incDec(thisPt, 0, crv->numPoints - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
                break;
                
              case ITEM_CURVE_XVAL:
                {
                  display.print(F("Xval:"));
                  display.setCursor(38, ypos);
                  display.print(crv->xVal[thisPt]);
                  if(isFocused)
                    markNode = true;
                  if(edit && thisPt > 0 && thisPt < crv->numPoints - 1)
                  {
                    int8_t _minVal = crv->xVal[thisPt - 1];
                    int8_t _maxVal = crv->xVal[thisPt + 1];
                    crv->xVal[thisPt] = incDec(crv->xVal[thisPt], _minVal, _maxVal, INCDEC_NOWRAP, INCDEC_NORMAL);
                  }
                }
                break;
                
              case ITEM_CURVE_YVAL:
                {
                  display.print(F("Yval:"));
                  display.setCursor(38, ypos);
                  display.print(crv->yVal[thisPt]);
                  if(isFocused)
                    markNode = true;
                  if(edit)
                    crv->yVal[thisPt] = incDec(crv->yVal[thisPt], -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                }
                break;
                
              case ITEM_CURVE_SMOOTH:
                {
                  display.print(F("Smth:"));
                  drawCheckbox(38, ypos, crv->smooth);
                  if(edit)
                    crv->smooth = incDec(crv->smooth, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
                }
                break;
            }
          }
          
          //scrollbar
          drawScrollBar(71, 19, ITEM_COUNT, topItem, 5, 5 * 9);
        
          //--- draw graph
          //draw stick input marker
          static int8_t lastVal = mixSources[Model.thrSrcRaw]/5;
          static uint32_t lastMovedTime = 0;
          bool moved = false;
          int16_t difference = mixSources[Model.thrSrcRaw]/5 - lastVal;
          if(difference >= 5 || difference <= -5)
          {
            lastVal = mixSources[Model.thrSrcRaw]/5;
            lastMovedTime = millis();
          }
          if(millis() - lastMovedTime < 3000)
          {
            moved = true;
            display.setInterlace(false);
          }
          //draw graph
          drawCustomCurve(crv, markNode ? thisPt : 0xff, moved ? Model.thrSrcRaw : (uint8_t)SRC_NONE);
        }

        //------ STICKS
        if(page == PAGE_STICKS)
        {
          display.setCursor(8, 9);
          strlcpy_P(textBuff, PSTR("Sticks"), sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

          uint8_t cntr = 0;
          for(uint8_t i = 0; i < NUM_STICK_AXES && i < 10; i++)
          {
            if(Sys.StickAxis[i].type == STICK_AXIS_ABSENT)
              continue;
            uint8_t xpos = 0;
            uint8_t ypos = 20 + cntr*9;
            if(cntr >= 5)
            {
              xpos = 69;
              ypos = 20 + (cntr-5)*9;
            }
            display.setCursor(xpos, ypos);
            getSrcName(textBuff, SRC_STICK_AXIS_FIRST + i, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(":"));
            display.setCursor(display.getCursorX() + 6, ypos);
            display.print(stickAxisIn[i]/5);
            cntr++;
          }
        }
        
        //------ KNOBS
        if(page == PAGE_KNOBS)
        {
          display.setCursor(8, 9);
          strlcpy_P(textBuff, PSTR("Knobs"), sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          for(uint8_t i = 0; i < NUM_KNOBS; i++)
          {
            if(Sys.Knob[i].type == KNOB_ABSENT)
              continue;
            uint8_t xpos = 0;
            uint8_t ypos = 20 + i*9;
            display.setCursor(xpos, ypos);
            getSrcName(textBuff, SRC_KNOB_FIRST + i, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(":"));
            display.setCursor(display.getCursorX() + 6, ypos);
            display.print(knobIn[i]/5);
          }
        }
        
        //------ SWITCHES
        if(page == PAGE_SWITCHES)
        {
          display.setCursor(8, 9);
          strlcpy_P(textBuff, PSTR("Switches"), sizeof(textBuff));
          display.print(textBuff);
          display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
          
          //show switches
          uint8_t cntr = 0;
          for(uint8_t i = 0; i < NUM_PHYSICAL_SWITCHES; i++)
          {
            if(Sys.swType[i] == SW_ABSENT)
              continue;
            
            uint8_t xpos = 0;
            uint8_t ypos = 20 + cntr * 9;
            if(cntr >= 5)
            {
              xpos = 69;
              ypos = 20 + (cntr - 5)*9;
            }
            display.setCursor(xpos, ypos);
            getSrcName(textBuff, SRC_SW_PHYSICAL_FIRST + i, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(": "));
            if(swState[i] == SWUPPERPOS) display.print(F("-100"));
            else if(swState[i] == SWLOWERPOS) display.print(F("100"));
            else display.print(F("0"));
            
            cntr++;
          }
        }
        
        //------ Exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MAIN_MENU);
      }
      break;
  }
}

#endif
