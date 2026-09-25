#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_home()
{
  switch(theScreen)
  {
    case SCREEN_HOME:
      {
        if(isRequestingSettingsRestore)
        {
          if(sdHasCard() && sdSystemSettingsExists())
          {
            changeToScreen(CONFIRMATION_RESTORE_SYSTEM_SETTINGS);
            break;
          }
          else
            isRequestingSettingsRestore = false;
        }

        if(isRequestingStickCalibration)
        {
          changeToScreen(SCREEN_STICKS);
          break; 
        }
        
        if(isRequestingKnobCalibration)
        {
          changeToScreen(SCREEN_KNOBS);
          break; 
        }

        if(isRequestingSwitchesSetup)
        {
          changeToScreen(SCREEN_SWITCHES);
          break;
        }
        
        if(!batteryGaugeCalibrated)
        {
          changeToScreen(SCREEN_BATTERY);
          break;
        }
        
        if((!Sys.lockStartup && !Sys.lockOnInactivity) || isEmptyStr(Sys.password, sizeof(Sys.password)))
          mainMenuLocked = false;

        //--------------- Icons -----------------
        
        uint8_t icon_xpos = 127;
        
        //Battery level indicator
        //Gauges don't indicate state of charge; only battery voltage
        if(Sys.useNumericalBatteryIndicator)
        {
          //print right aligned. 
          //Using a hack to measure the width of text first before actually printing.
          display.setCursor(0, 64);
          printVoltage(batteryVoltsNow);
          icon_xpos = 127 - display.getCursorX();
          display.setCursor(icon_xpos, 0);
          printVoltage(batteryVoltsNow);
        }
        else
        {
          icon_xpos = 113;
          display.drawRect(icon_xpos, 0, 14, 7, BLACK);
          display.drawVLine(icon_xpos + 14, 2, 3, BLACK);
          if(batteryState == BATTERY_HEALTHY)
          {
            static int8_t lastNumOfBars = 20;
            int8_t numOfBars = 2 + ((int32_t)(batteryVoltsNow - Sys.batteryVoltsMin) * 20) / (Sys.batteryVoltsMax - Sys.batteryVoltsMin);
            if(numOfBars > 20) 
              numOfBars = 20;
            if(numOfBars > lastNumOfBars && numOfBars - lastNumOfBars < 2) //prevent jitter at boundaries
              numOfBars = lastNumOfBars;
            for(uint8_t i = 0; i < numOfBars/2; i++)
              display.drawVLine(icon_xpos + 2 + i, 2, 3, BLACK);
            lastNumOfBars = numOfBars;
          }
        }
        
        //Draw other icons. Offset the x position by (4 + iconWidth)
        //Rf icon and tx power level as signal strength bars
        if(Sys.rfEnabled)
        {
          icon_xpos -= 17;
          display.drawBitmap(icon_xpos, 0, icon_rf, 7, 7, BLACK);
          uint8_t bars = 2 + (4 * Sys.rfPower) / (RF_POWER_COUNT - 1);
          for(uint8_t i = 0; i < bars; i++)
            display.drawVLine(icon_xpos + 6 + i, 6 - i, i + 1, BLACK);
        }

        //Mute icon
        if(!Sys.soundEnabled)
        {
          icon_xpos -= 13;
          display.drawBitmap(icon_xpos, 0, icon_mute, 8, 7, BLACK);
        }

        //Telemetry mute icon
        if(telemetryMuteAlarms && Sys.soundEnabled)
        {
          icon_xpos -= 19;
          display.drawBitmap(icon_xpos, 0, icon_telemetry_mute, 14, 7, BLACK);
        }

        //Lock icon
        if(mainMenuLocked)
        {
          icon_xpos -= 10;
          display.drawBitmap(icon_xpos, 0, icon_padlock, 5, 7, BLACK);
        }

        //------------ Trims --------------------
        static uint32_t endTime; 
        if(buttonCode >= KEY_TRIM_FIRST && buttonCode <= KEY_TRIM_LAST)
          endTime = millis() + 3000;
        if(!Sys.autohideTrims || (Sys.autohideTrims && millis() < endTime) || isOnscreenTrimMode)
          drawTrimSliders();

        //------------ Model name, flight modes -------------
        //Model name
        display.setCursor(11, 9);
        printModelName(Model.name, Sys.activeModelIdx);

        //Flightmode
        //Determine if flight modes are defined other than the default. 
        //If any exist, show the active flight mode. Otherwise hide it, as user does not care.
        bool hasDefinedFlightModes = false;
        for(uint8_t i = 1; i < NUM_FLIGHT_MODES; i++)
        {
          if(Model.FlightMode[i].swtch != CTRL_SW_NONE)
          {
            hasDefinedFlightModes = true;
            break;
          }
        }
        if(hasDefinedFlightModes)
        {
          if(isEmptyStr(Model.FlightMode[activeFmdIdx].name, sizeof(Model.FlightMode[0].name)))
            getControlSwitchName(textBuff, CTRL_SW_FMD_FIRST + activeFmdIdx, sizeof(textBuff));
          else
            strlcpy(textBuff, Model.FlightMode[activeFmdIdx].name, sizeof(textBuff));
          //print right aligned
          uint8_t len = strlen(textBuff) + 2; //add 2 for brackets 
          display.setCursor(118 - len * 6, 9);
          display.print(F("("));
          display.print(textBuff);
          display.print(F(")"));
        }
        
        //separator line
        display.drawHLine(11, 18, 106, BLACK);
        
        //------------ Widgets ------------------
        
        //get available telemetry sensors into a list
        uint8_t tlmQQ[NUM_WIDGETS];
        uint8_t tlmCount = 0;
        for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
        {
          if(!isEmptyStr(Model.Telemetry[i].name, sizeof(Model.Telemetry[0].name))) 
          {
            //skip sensors that are not set to show on home
            if(!Model.Telemetry[i].showOnHome)
              continue;
            //skip "no data" sensors
            if(telemetryReceivedValue[i] == TELEMETRY_NO_DATA) 
              continue;
            //skip special telemetry
            if(Model.Telemetry[i].type == TELEMETRY_TYPE_GNSS)
              continue;
            //check if already assigned to a widget
            bool skip = false;
            for(uint8_t j = 0; j < NUM_WIDGETS; j++)
            {
              if(Model.Widget[j].type == WIDGET_TYPE_TELEMETRY && Model.Widget[j].src == i)
                skip = true;
            }
            if(skip)
              continue;
            
            tlmQQ[tlmCount] = i;
            tlmCount++;
            if(tlmCount == sizeof(tlmQQ))
              break;
          }
        }
        
        //draw the widgets
        uint8_t ypos = 21;
        uint8_t printCount = 0;
        uint8_t tlmCntr = 0;
        for(uint8_t i = 0; i < NUM_WIDGETS; i++)
        {
          widget_params_t *widget = &Model.Widget[i];
          bool hasPrinted = false;
          if(widget->type == WIDGET_TYPE_TELEMETRY)
          {
            uint8_t idx = widget->src;
            if(widget->src == WIDGET_SRC_AUTO)
            { 
              if(tlmCount == 0 || tlmCntr >= tlmCount)
                continue;
              idx = tlmQQ[tlmCntr];
              tlmCntr++;
            }
            display.setCursor(11, ypos);
            display.print(Model.Telemetry[idx].name);
            display.print(F(":"));
            if(widget->disp == WIDGET_DISP_NUMERICAL || telemetryReceivedValue[idx] == TELEMETRY_NO_DATA)
              drawTelemetryValue(67, ypos, idx, telemetryReceivedValue[idx], true);
            else if(widget->disp == WIDGET_DISP_GAUGE)
            {
              bool show = true;
              if(telemetryAlarmState[idx] && (millis() % 1000 > 700))
                show = false;
              if(show)
              {
                int32_t tVal = ((int32_t) telemetryReceivedValue[idx] * Model.Telemetry[idx].multiplier) / 100;
                tVal += Model.Telemetry[idx].offset;
                drawHorizontalBarChart(70, ypos + 1, 41, 4, BLACK, tVal, widget->gaugeMin, widget->gaugeMax);
              }
            }
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_MIXSOURCES)
          {
            if(widget->src == SRC_NONE)
              continue;
            display.setCursor(11, ypos);
            getSrcName(textBuff, widget->src, sizeof(textBuff));
            display.print(textBuff);
            display.print(F(":"));
            display.setCursor(67, ypos);
            if(widget->disp == WIDGET_DISP_NUMERICAL)
              display.print(mixSources[widget->src]/5);
            else if(widget->disp == WIDGET_DISP_GAUGE)
              drawHorizontalBarChart(70, ypos + 1, 41, 4, BLACK, mixSources[widget->src]/5, widget->gaugeMin, widget->gaugeMax);
            else if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
              drawHorizontalBarChartZeroCentered(71, ypos + 2, 39, 3, BLACK, mixSources[widget->src]/5, widget->gaugeMax - widget->gaugeMin);
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_OUTPUTS)
          {
            display.setCursor(11, ypos);
            if(!isEmptyStr(Model.Channel[widget->src].name, sizeof(Model.Channel[0].name)))
              display.print(Model.Channel[widget->src].name);
            else
            {
              getSrcName(textBuff, SRC_CH1 + widget->src, sizeof(textBuff));
              display.print(textBuff);
              display.print(F(" out"));
            }
            display.print(F(":"));
            display.setCursor(67, ypos);
            if(widget->disp == WIDGET_DISP_NUMERICAL)
              display.print(channelOut[widget->src]/5);
            else if(widget->disp == WIDGET_DISP_GAUGE)
              drawHorizontalBarChart(70, ypos + 1, 41, 4, BLACK, channelOut[widget->src]/5, widget->gaugeMin, widget->gaugeMax);
            else if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
              drawHorizontalBarChartZeroCentered(71, ypos + 2, 39, 3, BLACK, channelOut[widget->src]/5, widget->gaugeMax - widget->gaugeMin);
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_TIMERS)
          {
            display.setCursor(11, ypos);
            printTimerValue(widget->src);
            //print timer name
            uint8_t name_xpos = display.getCursorX() + 6;
            if(name_xpos < 67)
              name_xpos = 67;
            display.setCursor(name_xpos, ypos);
            display.print(Model.Timer[widget->src].name);
            hasPrinted = true;
          }
          else if(widget->type == WIDGET_TYPE_COUNTERS)
          {
            display.setCursor(11, ypos);
            if(!isEmptyStr(Model.Counter[widget->src].name, sizeof(Model.Counter[0].name)))
              display.print(Model.Counter[widget->src].name);
            else
            {
              getSrcName(textBuff, SRC_COUNTER_FIRST + widget->src, sizeof(textBuff));
              display.print(textBuff);
            }
            display.print(F(":"));
            display.setCursor(67, ypos);
            display.print(counterOut[widget->src]);
            hasPrinted = true;
          }

          if(hasPrinted)
          {
            printCount++;
            ypos += 9;
          }
          //abort if no more space on the screen
          if(printCount == 4)
            break;
        }
        
        //-------------- handle keys ------------

        if(isOnscreenTrimMode)
        {
          if(heldButton == KEY_SELECT)
          {
            isOnscreenTrimMode = false;
            killButtonEvents();
            audioToPlay = AUDIO_TRIM_MODE_EXITED;
          }

          if(pressedButton == KEY_SELECT)
          {
            //suppress AUDIO_KEY_PRESSED
            if(audioToPlay == AUDIO_KEY_PRESSED)
              audioToPlay = AUDIO_NONE;
          }

          if(clickedButton == KEY_SELECT)
          {
            trimIdx++;
            if(trimIdx > 3)
              trimIdx = 0;
            audioToPlay = AUDIO_TRIM_MODE_X1 + trimIdx;
          }

          flight_mode_t* fmd = &Model.FlightMode[activeFmdIdx];
          
          //Adjust trim values
          switch(trimIdx)
          {
            case 0:
              //x1 axis
              if(Model.X1Trim.trimState == TRIM_COMMON)
                Model.X1Trim.commonTrim = adjustTrim(0, Model.X1Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.X1Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->x1Trim = adjustTrim(0, fmd->x1Trim, KEY_UP, KEY_DOWN);
              break;
              
            case 1:
              //y1 axis
              if(Model.Y1Trim.trimState == TRIM_COMMON)
                 Model.Y1Trim.commonTrim = adjustTrim(1, Model.Y1Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.Y1Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->y1Trim = adjustTrim(1, fmd->y1Trim, KEY_UP, KEY_DOWN);
              break;
            
            case 2:
              //x2 axis
              if(Model.X2Trim.trimState == TRIM_COMMON)
                Model.X2Trim.commonTrim = adjustTrim(2, Model.X2Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.X2Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->x2Trim = adjustTrim(2, fmd->x2Trim, KEY_UP, KEY_DOWN);
              break;
              
            case 3:
              //y2 axis
              if(Model.Y2Trim.trimState == TRIM_COMMON)
                 Model.Y2Trim.commonTrim = adjustTrim(3, Model.Y2Trim.commonTrim, KEY_UP, KEY_DOWN);
              else if(Model.Y2Trim.trimState == TRIM_FLIGHT_MODE)
                fmd->y2Trim = adjustTrim(3, fmd->y2Trim, KEY_UP, KEY_DOWN);
              break;
          }
        }
        else
        {
          if(heldButton == KEY_UP)
          {
            isOnscreenTrimMode = true;
            killButtonEvents();
            audioToPlay = AUDIO_TRIM_MODE_ENTERED;
          }

          if(clickedButton == KEY_SELECT)
          {
            if(mainMenuLocked) 
              changeToScreen(SCREEN_UNLOCK_MAIN_MENU);
            else 
              changeToScreen(SCREEN_MAIN_MENU);
          }

          if(clickedButton == KEY_UP)
            changeToScreen(SCREEN_CHANNEL_MONITOR);

          if(clickedButton == KEY_DOWN)
            changeToScreen(CONTEXT_MENU_HOME_SCREEN);
          
          //mute audible telemetry alarms
          if(heldButton == KEY_DOWN && millis() - buttonStartTime > 1000)
          {
            killButtonEvents();
            telemetryMuteAlarms = !telemetryMuteAlarms;
            audioToPlay = AUDIO_TELEM_MUTE_CHANGED;
            makeToast(telemetryMuteAlarms ? PSTR("Telemetry muted") : PSTR("Telemetry unmuted") , 2000, 0);
          }
        }
      }
      break;
      
    case SCREEN_CHANNEL_MONITOR:
      {
        // display.setInterlace(false); 
        
        drawHeader(PSTR("Outputs"));
        
        //scrollable 
        uint8_t numPages = (NUM_RC_CHANNELS + 9) / 10;
        static uint8_t thisPage = 1;
        
        static bool viewInitialised = false;
        if(!viewInitialised) 
        {
          if(lastScreen == CONTEXT_MENU_OUTPUTS) //start in the page that has the channel we want to view
            thisPage = (thisChIdx + 10) / 10;
          else
            thisPage = 1;
          viewInitialised = true;
        }

        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);
        
        uint8_t startIdx = (thisPage - 1) * 10;
        for(uint8_t i = startIdx; i < startIdx + 10 && i < NUM_RC_CHANNELS; i++)
        {
          if((i - startIdx) < 5)
            display.setCursor(0, 10 + (i - startIdx) * 11);
          else
            display.setCursor(66, 10 + (i - (startIdx + 5)) * 11);
          display.print(F("Ch"));
          display.print(1 + i);  
          if(lastScreen == CONTEXT_MENU_OUTPUTS && i == thisChIdx)
            display.write(0xB1);
          else
            display.print(F(":"));
          if(i < 9)
            display.print(F(" "));
          // display.print(channelOut[i] / 5);
          //show as decimal value
          int16_t val = channelOut[i];
          display.print(val / 5);
          val = abs(val);
          if(val < 500)
          {
            display.print(F("."));
            display.print((val % 5) * 2);
          }
        }
        //show scrollbar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          if(lastScreen == CONTEXT_MENU_OUTPUTS)
            changeToScreen(SCREEN_OUTPUTS);
          else
            changeToScreen(SCREEN_HOME);
        }
      }
      break;
      
    case CONTEXT_MENU_HOME_SCREEN:
      {
        enum {
          ITEM_SETUP_WIDGETS,
          ITEM_START_STOP_TIMER_FIRST,
          ITEM_START_STOP_TIMER_LAST = ITEM_START_STOP_TIMER_FIRST + NUM_TIMERS - 1,
          ITEM_RESET_TIMER_FIRST,
          ITEM_RESET_TIMER_LAST = ITEM_RESET_TIMER_FIRST + NUM_TIMERS - 1,
        };
        
        static char const startTimerStr[][20] PROGMEM = {"Start timer 1", "Start timer 2", "Start timer 3"};
        static char const stopTimerStr[][20] PROGMEM = {"Stop timer 1", "Stop timer 2", "Stop timer 3"};
        static char const resetTimerStr[][20] PROGMEM = {"Reset timer 1", "Reset timer 2", "Reset timer 3"};

        //dynamically add items
        contextMenuInitialise();
        for(uint8_t idx = 0; idx < NUM_TIMERS; idx++)
        {
          //only add a timer if the timer has a widget associated
          for(uint8_t i = 0; i < NUM_WIDGETS; i++)
          {
            if(Model.Widget[i].type == WIDGET_TYPE_TIMERS && Model.Widget[i].src == idx)
            {
              if(Model.Timer[idx].swtch == CTRL_SW_NONE)
              {
                if(timerIsRunning[idx]) 
                  contextMenuAddItem(stopTimerStr[idx], ITEM_START_STOP_TIMER_FIRST + idx);
                else 
                  contextMenuAddItem(startTimerStr[idx], ITEM_START_STOP_TIMER_FIRST + idx);
              }
              contextMenuAddItem(resetTimerStr[idx], ITEM_RESET_TIMER_FIRST + idx);
              break; 
            }
          }
        }
        if(!mainMenuLocked)        
          contextMenuAddItem(PSTR("Set up widgets"), ITEM_SETUP_WIDGETS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID >= ITEM_START_STOP_TIMER_FIRST && contextMenuSelectedItemID <= ITEM_START_STOP_TIMER_LAST)
        {
          uint8_t idx = contextMenuSelectedItemID - ITEM_START_STOP_TIMER_FIRST;
          timerForceRun[idx] = timerIsRunning[idx] ? false : true;
          changeToScreen(SCREEN_HOME);
        }
        if(contextMenuSelectedItemID >= ITEM_RESET_TIMER_FIRST && contextMenuSelectedItemID <= ITEM_RESET_TIMER_LAST)
        {
          uint8_t idx = contextMenuSelectedItemID - ITEM_RESET_TIMER_FIRST;
          resetTimerRegister(idx);
          changeToScreen(SCREEN_HOME);
        }
        if(contextMenuSelectedItemID == ITEM_SETUP_WIDGETS)
          changeToScreen(SCREEN_WIDGET_SETUP);
          
        if(heldButton == KEY_SELECT || contextMenuGetItemCount() == 0) //exit
          changeToScreen(SCREEN_HOME);
      }
      break;

    case SCREEN_WIDGET_SETUP:
      {
        drawHeader(PSTR("Widgets"));
        
        widget_params_t *widget = &Model.Widget[thisWidgetIdx];
        
        display.setCursor(8, 9);
        display.print(F("Widget"));
        display.print(thisWidgetIdx + 1);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);

        //dynamic list
        enum {
          ITEM_TYPE,
          ITEM_SRC,
          ITEM_DISP,
          ITEM_GAUGE_MIN,
          ITEM_GAUGE_MAX,

          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT];
        uint8_t listItemCount = 0;

        //add items to list
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(widget->type == WIDGET_TYPE_TIMERS 
            || widget->type == WIDGET_TYPE_COUNTERS
            || (widget->type == WIDGET_TYPE_TELEMETRY && widget->src == WIDGET_SRC_AUTO))
          {
            if(i == ITEM_DISP || i == ITEM_GAUGE_MIN || i == ITEM_GAUGE_MAX)
              continue;
          }
          if(widget->disp == WIDGET_DISP_NUMERICAL)
          {
            if(i == ITEM_GAUGE_MIN || i == ITEM_GAUGE_MAX)
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
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(40, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (focusedItem > 1 && focusedItem != numFocusable && itemID == listItemIDs[focusedItem - 2] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_TYPE:
              {
                display.print(F("Type:"));
                display.setCursor(48, ypos);
                display.print(findStringInIdStr(enum_WidgetType, widget->type));
                if(edit)
                {
                  uint8_t prevType = widget->type;
                  widget->type = incDec(widget->type, 0, WIDGET_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  if(widget->type != prevType)
                  {
                    widget->disp = WIDGET_DISP_NUMERICAL;
                    widget->gaugeMin = -100;
                    widget->gaugeMax = 100;
                    if(widget->type == WIDGET_TYPE_TELEMETRY)
                    {
                      widget->src = WIDGET_SRC_AUTO;
                      widget->gaugeMin = 0;
                    }
                    if(widget->type == WIDGET_TYPE_MIXSOURCES)
                      widget->src = SRC_NONE;
                    if(widget->type == WIDGET_TYPE_OUTPUTS 
                      || widget->type == WIDGET_TYPE_COUNTERS 
                      || widget->type == WIDGET_TYPE_TIMERS)
                    {
                      widget->src = 0;
                    }
                  }
                }
              }
              break;
            
            case ITEM_SRC:
              {
                display.print(F("Src:"));
                display.setCursor(48, ypos);
                if(widget->type == WIDGET_TYPE_TELEMETRY)
                {
                  if(widget->src == WIDGET_SRC_AUTO) 
                    display.print(findStringInIdStr(enum_WidgetSource, WIDGET_SRC_AUTO));
                  else 
                    display.print(Model.Telemetry[widget->src].name);
                  if(edit)
                  {
                    //skip empty telemetry (non-extant sensors)
                    //skip special telemetry
                    //use an array to hold extant sensors
                    uint8_t srcQQ[NUM_CUSTOM_TELEMETRY + 1]; //1 added for auto
                    uint8_t srcCount = 0;
                    for(uint8_t i = 0; i < sizeof(srcQQ) - 1; i++)
                    {
                      if(isEmptyStr(Model.Telemetry[i].name, sizeof(Model.Telemetry[0].name)))
                        continue;
                      if(Model.Telemetry[i].type == TELEMETRY_TYPE_GNSS)
                        continue;
                      srcQQ[srcCount] = i;
                      srcCount++;
                    }
                    srcQQ[srcCount] = WIDGET_SRC_AUTO;
                    srcCount++;
        
                    uint8_t idxQQ = 0;
                    for(uint8_t i = 0; i < srcCount; i++) //search for a match
                    {
                      if(srcQQ[i] == widget->src)
                      {
                        idxQQ = i;
                        break;
                      }
                    }
                    idxQQ = incDec(idxQQ, 0, srcCount - 1, INCDEC_WRAP, INCDEC_SLOW);
                    uint8_t prevSrc = widget->src;
                    widget->src = srcQQ[idxQQ];
                    if(widget->src != prevSrc)
                    {
                      widget->gaugeMin = 0;
                      widget->gaugeMax = 100;
                    }
                  }
                }
                else if(widget->type == WIDGET_TYPE_OUTPUTS)
                {
                  getSrcName(textBuff, SRC_CH1 + widget->src, sizeof(textBuff));
                  display.print(textBuff);
                  if(!isEmptyStr(Model.Channel[widget->src].name, sizeof(Model.Channel[0].name)))
                  {
                    display.print(F(" "));
                    display.print(Model.Channel[widget->src].name);
                  }
                  if(edit)
                    widget->src = incDec(widget->src, 0, NUM_RC_CHANNELS - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
                else if(widget->type == WIDGET_TYPE_MIXSOURCES)
                {
                  getSrcName(textBuff, widget->src, sizeof(textBuff));
                  display.print(textBuff);
                  if(edit)
                  {
                    //detect moved source
                    uint8_t movedSrc = procMovedSource(getMovedSource());
                    if(movedSrc != SRC_NONE)
                      widget->src = movedSrc;
                    //inc dec
                    widget->src = incDecSource(widget->src, INCDEC_FLAG_MIX_SRC);
                  }
                }
                else if(widget->type == WIDGET_TYPE_COUNTERS)
                {
                  if(!isEmptyStr(Model.Counter[widget->src].name, sizeof(Model.Counter[0].name)))
                    display.print(Model.Counter[widget->src].name);
                  else
                  {
                    getSrcName(textBuff, SRC_COUNTER_FIRST + widget->src, sizeof(textBuff));
                    display.print(textBuff);
                  }
                  if(edit)
                    widget->src = incDec(widget->src, 0, NUM_COUNTERS - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
                else if(widget->type == WIDGET_TYPE_TIMERS)
                {
                  getSrcName(textBuff, SRC_TIMER_FIRST + widget->src, sizeof(textBuff));
                  display.print(textBuff);
                  if(!isEmptyStr(Model.Timer[widget->src].name, sizeof(Model.Timer[0].name)))
                  {
                    display.print(F(" "));
                    display.print(Model.Timer[widget->src].name);
                  }
                  if(edit)
                    widget->src = incDec(widget->src, 0, NUM_TIMERS - 1, INCDEC_WRAP, INCDEC_SLOW);
                }
              }
              break;
            
            case ITEM_DISP:
              {
                display.print(F("Disp:"));
                display.setCursor(48, ypos);
                display.print(findStringInIdStr(enum_WidgetDisplay, widget->disp));
                if(edit && (buttonCode == KEY_UP || buttonCode == KEY_DOWN))
                {
                  uint8_t prevDisp = widget->disp;
                  do {
                    widget->disp = incDec(widget->disp, 0, WIDGET_DISP_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
                  } while(widget->type == WIDGET_TYPE_TELEMETRY && widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED);
                  
                  if(widget->disp != prevDisp)
                  {
                    if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
                    {
                      if(widget->gaugeMin > 0)
                        widget->gaugeMin = 0 - widget->gaugeMin;
                      widget->gaugeMax = 0 - widget->gaugeMin;
                    }
                  }
                }
              }
              break;
            
            case ITEM_GAUGE_MIN:
              {
                display.print((widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED) ? F("Left:") : F("Min:"));
                display.setCursor(48, ypos);
                if(widget->type == WIDGET_TYPE_TELEMETRY)
                {
                  display.setCursor(display.getCursorX(), display.getCursorY());
                  printTelemParam(widget->src, widget->gaugeMin, true);
                }
                else if(widget->type == WIDGET_TYPE_OUTPUTS || widget->type == WIDGET_TYPE_MIXSOURCES)
                  display.print(widget->gaugeMin);
                if(edit)
                {
                  if(widget->type == WIDGET_TYPE_TELEMETRY)
                    widget->gaugeMin = incDec(widget->gaugeMin, -30000, widget->gaugeMax, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                  else if(widget->type == WIDGET_TYPE_MIXSOURCES || widget->type == WIDGET_TYPE_OUTPUTS)
                    widget->gaugeMin = incDec(widget->gaugeMin, -100, widget->gaugeMax, INCDEC_NOWRAP, INCDEC_NORMAL);
                  if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
                    widget->gaugeMax = 0 - widget->gaugeMin;
                }
              }
              break;
            
            case ITEM_GAUGE_MAX:
              {
                display.print((widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED) ? F("Right:") : F("Max:"));
                display.setCursor(48, ypos);
                if(widget->type == WIDGET_TYPE_TELEMETRY)
                {
                  display.setCursor(display.getCursorX(), display.getCursorY());
                  printTelemParam(widget->src, widget->gaugeMax, true);
                }
                else if(widget->type == WIDGET_TYPE_OUTPUTS || widget->type == WIDGET_TYPE_MIXSOURCES)
                  display.print(widget->gaugeMax);
                if(edit)
                {
                  if(widget->type == WIDGET_TYPE_TELEMETRY)
                    widget->gaugeMax = incDec(widget->gaugeMax, widget->gaugeMin, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
                  else if(widget->type == WIDGET_TYPE_MIXSOURCES || widget->type == WIDGET_TYPE_OUTPUTS)
                    widget->gaugeMax = incDec(widget->gaugeMax, widget->gaugeMin, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
                  if(widget->disp == WIDGET_DISP_GAUGE_ZERO_CENTERED)
                    widget->gaugeMin = 0 - widget->gaugeMax;
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
          changeToScreen(CONTEXT_MENU_WIDGETS);
        
        //change to next widget
        if(focusedItem == 1)
        {          
          drawCursor(0, 9);
          thisWidgetIdx = incDec(thisWidgetIdx, 0, NUM_WIDGETS - 1, INCDEC_WRAP, INCDEC_SLOW);
        }

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_HOME);
      }
      break;
      
    case CONTEXT_MENU_WIDGETS:
      {
        enum {
          ITEM_COPY_WIDGET,
          ITEM_MOVE_WIDGET,
          ITEM_RESET_WIDGET,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Copy to"), ITEM_COPY_WIDGET);
        contextMenuAddItem(PSTR("Move to"), ITEM_MOVE_WIDGET);
        contextMenuAddItem(PSTR("Reset widget"), ITEM_RESET_WIDGET);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RESET_WIDGET)
        {
          resetWidgetParams(thisWidgetIdx);
          changeToScreen(SCREEN_WIDGET_SETUP);
        }
        if(contextMenuSelectedItemID == ITEM_COPY_WIDGET)
        {
          destWidgetIdx = thisWidgetIdx;
          changeToScreen(DIALOG_COPY_WIDGET);
        }
        if(contextMenuSelectedItemID == ITEM_MOVE_WIDGET)
        {
          destWidgetIdx = thisWidgetIdx;
          changeToScreen(DIALOG_MOVE_WIDGET);
        }
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_WIDGET_SETUP);
      }
      break;
      
    case DIALOG_COPY_WIDGET:
    case DIALOG_MOVE_WIDGET:
      {
        isEditMode = true;
        destWidgetIdx = incDec(destWidgetIdx, 0, NUM_WIDGETS - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Widget"), thisWidgetIdx, destWidgetIdx, theScreen == DIALOG_COPY_WIDGET);
        if(clickedButton == KEY_SELECT)
        {
          if(theScreen == DIALOG_COPY_WIDGET)
            Model.Widget[destWidgetIdx] = Model.Widget[thisWidgetIdx];
          else
          {
            uint8_t newPos = destWidgetIdx;
            uint8_t oldPos = thisWidgetIdx;
            //copy from old position to temp
            widget_params_t Temp_Widget = Model.Widget[oldPos];
            //shift elements
            uint8_t thisPos = oldPos;
            if(newPos < oldPos)
            {
              while(thisPos > newPos)
              {
                Model.Widget[thisPos] = Model.Widget[thisPos - 1];
                thisPos--;
              }
            }
            else if(newPos > oldPos)
            {
              while(thisPos < newPos)
              {
                Model.Widget[thisPos] = Model.Widget[thisPos + 1];
                thisPos++;
              }
            }
            //copy from temp into new position
            Model.Widget[newPos] = Temp_Widget;
          }
          thisWidgetIdx = destWidgetIdx;
          changeToScreen(SCREEN_WIDGET_SETUP);
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_WIDGET_SETUP);
      }
      break;
  }
}

#endif
