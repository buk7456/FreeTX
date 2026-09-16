#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_telemetry()
{
  switch(theScreen)
  {
    case SCREEN_TELEMETRY:
      {
        drawHeader(mainMenu[MAIN_MENU_TELEMETRY]);
        
        //--- scrollable list ----
        
        static uint8_t topItem = 1;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          focusedItem = thisTelemIdx + 1;
          viewInitialised = true;
        }
        
        //handle navigation
        uint8_t numFocusable = NUM_CUSTOM_TELEMETRY;
        changeFocusOnUpDown(numFocusable);
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 6)
          topItem++;
        
        //fill list
        for(uint8_t line = 0; line < 6 && line < NUM_CUSTOM_TELEMETRY; line++)
        {
          uint8_t ypos = 10 + line * 9;
          uint8_t item = topItem + line;

          if(focusedItem == topItem + line) //highlight
          {
            display.fillRect(0, ypos - 1, 7, 9, BLACK);
            display.fillRect(2, ypos + 2, 3, 3, WHITE);
          }
          else
            display.fillRect(2, ypos + 2, 3, 3, BLACK);

          uint8_t idx = item - 1;
          if(!isEmptyStr(Model.Telemetry[idx].name, sizeof(Model.Telemetry[0].name)))
          {
            display.setCursor(11, ypos);
            display.print(Model.Telemetry[idx].name);
            display.print(F(":"));
            if(Model.Telemetry[idx].type == TELEMETRY_TYPE_GENERAL)
            {
              drawTelemetryValue(67, ypos, idx, telemetryReceivedValue[idx], true);
            }
            else if(Model.Telemetry[idx].type == TELEMETRY_TYPE_GNSS)
            {
              display.setCursor(67, ypos);
              if(GNSSTelemetryData.satellitesInView > 0)
              {
                display.print(GNSSTelemetryData.satellitesInUse);
                display.print(F("/"));
                display.print(GNSSTelemetryData.satellitesInView);
                display.print(F(" sat"));
              }
              else
                display.print(F("No data"));
            }
          }
        }

        //scroll bar
        drawScrollBar(127, 9, NUM_CUSTOM_TELEMETRY, topItem, 6, 6 * 9);

        //open sensor context menu
        if(focusedItem <= NUM_CUSTOM_TELEMETRY)
        {
          thisTelemIdx = focusedItem - 1;
          if(clickedButton == KEY_SELECT)
          {
            viewInitialised = false;
            if(isEmptyStr(Model.Telemetry[thisTelemIdx].name, sizeof(Model.Telemetry[0].name)))
              changeToScreen(CONTEXT_MENU_FREE_SENSOR);
            else
              changeToScreen(CONTEXT_MENU_ACTIVE_SENSOR);
          }
        }

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_MAIN_MENU);
        }
      }
      break;

    case CONTEXT_MENU_FREE_SENSOR:
      {
        enum {
          ITEM_NEW_SENSOR,
          ITEM_SENSOR_TEMPLATES
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("New sensor"), ITEM_NEW_SENSOR);
        contextMenuAddItem(PSTR("Templates"), ITEM_SENSOR_TEMPLATES);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_NEW_SENSOR)
        { 
          resetTelemParams(thisTelemIdx);        
          changeToScreen(SCREEN_CREATE_SENSOR);
        }
        if(contextMenuSelectedItemID == ITEM_SENSOR_TEMPLATES)
        {
          resetTelemParams(thisTelemIdx);
          changeToScreen(CONTEXT_MENU_SENSOR_TEMPLATES);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case SCREEN_CREATE_SENSOR:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Sensor name"), Model.Telemetry[thisTelemIdx].name, sizeof(Model.Telemetry[0].name), 
                       false, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_EDIT_SENSOR);
      } 
      break;
      
    case CONTEXT_MENU_SENSOR_TEMPLATES:
      {
        enum {
          ITEM_EXTVOLTS_2S,
          ITEM_EXTVOLTS_3S,
          ITEM_EXTVOLTS_4S,
          ITEM_RSSI,
          ITEM_LINK_QUALITY,
          ITEM_GNSS,
          ITEM_GNSS_DISTANCE,
          ITEM_GNSS_SPEED,
          ITEM_GNSS_AGL_ALTITUDE,
          ITEM_GNSS_MSL_ALTITUDE,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("External volts 2S"), ITEM_EXTVOLTS_2S);
        contextMenuAddItem(PSTR("External volts 3S"), ITEM_EXTVOLTS_3S);
        contextMenuAddItem(PSTR("External volts 4S"), ITEM_EXTVOLTS_4S);
        contextMenuAddItem(PSTR("Link quality"), ITEM_LINK_QUALITY);
        contextMenuAddItem(PSTR("RSSI"), ITEM_RSSI);
        bool isGNSSAlreadyAdded = false;
        for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
        {
          if(Model.Telemetry[i].type == TELEMETRY_TYPE_GNSS)
          {
            isGNSSAlreadyAdded = true;
            contextMenuAddItem(PSTR("GNSS distance"), ITEM_GNSS_DISTANCE);
            contextMenuAddItem(PSTR("GNSS speed"), ITEM_GNSS_SPEED);
            contextMenuAddItem(PSTR("GNSS altitude AGL"), ITEM_GNSS_AGL_ALTITUDE);
            contextMenuAddItem(PSTR("GNSS altitude MSL"), ITEM_GNSS_MSL_ALTITUDE);
            break;
          }
        }
        if(!isGNSSAlreadyAdded)
          contextMenuAddItem(PSTR("GNSS/GPS"), ITEM_GNSS);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_EXTVOLTS_2S)  loadSensorTemplateExtVolts2S(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_EXTVOLTS_3S)  loadSensorTemplateExtVolts3S(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_EXTVOLTS_4S)  loadSensorTemplateExtVolts4S(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_RSSI)         loadSensorTemplateRSSI(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_LINK_QUALITY) loadSensorTemplateLinkQuality(thisTelemIdx);

        if(contextMenuSelectedItemID == ITEM_GNSS)         
        {
          loadSensorTemplateGNSS(thisTelemIdx);
          if(Sys.defaultGnssUnits == GNSS_DEFAULT_UNITS_NONE)
            changeToScreen(SCREEN_SELECT_GNSS_UNITS);
          else
          {
            loadDefaultGnssUnits();
            changeToScreen(SCREEN_TELEMETRY);
          }
        }

        if(contextMenuSelectedItemID == ITEM_GNSS_SPEED)   loadSensorTemplateGNSSSpeed(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS_DISTANCE)  loadSensorTemplateGNSSDistance(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS_AGL_ALTITUDE)  loadSensorTemplateGNSSAGLAltitude(thisTelemIdx);
        if(contextMenuSelectedItemID == ITEM_GNSS_MSL_ALTITUDE)  loadSensorTemplateGNSSMSLAltitude(thisTelemIdx);
        
        if(contextMenuSelectedItemID != 0xff && contextMenuSelectedItemID != ITEM_GNSS) 
          changeToScreen(SCREEN_TELEMETRY);
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case CONTEXT_MENU_ACTIVE_SENSOR:
      {
        enum {
          ITEM_VIEW_STATISTICS,
          ITEM_EDIT_SENSOR,
          ITEM_DELETE_SENSOR,
          ITEM_VIEW_GNSS_DATA,
          ITEM_RESET_AGL_ALTITUDE,
          ITEM_RESET_LAST_KNOWN_LOCATION,
          ITEM_RESET_STARTING_POINT,
          ITEM_CHANGE_GNSS_UNITS,
        };
        
        contextMenuInitialise();
        if(Model.Telemetry[thisTelemIdx].type == TELEMETRY_TYPE_GENERAL)
        {
          contextMenuAddItem(PSTR("View statistics"), ITEM_VIEW_STATISTICS);
          contextMenuAddItem(PSTR("Edit sensor"), ITEM_EDIT_SENSOR);
          contextMenuAddItem(PSTR("Delete sensor"), ITEM_DELETE_SENSOR);
          if(Model.Telemetry[thisTelemIdx].identifier == SENSOR_ID_GNSS_DISTANCE)
            contextMenuAddItem(PSTR("Reset starting point"), ITEM_RESET_STARTING_POINT);
          if(Model.Telemetry[thisTelemIdx].identifier == SENSOR_ID_GNSS_AGL_ALTITUDE)
            contextMenuAddItem(PSTR("Reset altitude AGL"), ITEM_RESET_AGL_ALTITUDE);
        }
        else if(Model.Telemetry[thisTelemIdx].type == TELEMETRY_TYPE_GNSS)
        {
          contextMenuAddItem(PSTR("View GNSS data"), ITEM_VIEW_GNSS_DATA);
          contextMenuAddItem(PSTR("Reset starting point"), ITEM_RESET_STARTING_POINT);
          contextMenuAddItem(PSTR("Reset altitude AGL"), ITEM_RESET_AGL_ALTITUDE);
          contextMenuAddItem(PSTR("Reset last known location"), ITEM_RESET_LAST_KNOWN_LOCATION);
          // Add "Delete sensor" option if all the child items have been deleted. Same behaviour for "Change units" option.
          bool hasChildren = false;
          for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
          {
            uint8_t id = Model.Telemetry[i].identifier;
            if(id == SENSOR_ID_GNSS_DISTANCE
              || id == SENSOR_ID_GNSS_SPEED
              || id == SENSOR_ID_GNSS_AGL_ALTITUDE
              || id == SENSOR_ID_GNSS_MSL_ALTITUDE)
            {
              hasChildren = true;
              break;
            }
          }
          if(!hasChildren)
          {
            contextMenuAddItem(PSTR("Delete sensor"), ITEM_DELETE_SENSOR);
            contextMenuAddItem(PSTR("Change units"), ITEM_CHANGE_GNSS_UNITS);
          }
        }
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_VIEW_STATISTICS) changeToScreen(SCREEN_SENSOR_STATISTICS);
        if(contextMenuSelectedItemID == ITEM_EDIT_SENSOR) changeToScreen(SCREEN_EDIT_SENSOR);
        if(contextMenuSelectedItemID == ITEM_DELETE_SENSOR) changeToScreen(CONFIRMATION_DELETE_SENSOR);
        if(contextMenuSelectedItemID == ITEM_VIEW_GNSS_DATA) changeToScreen(SCREEN_TELEMETRY_GNSS);
        if(contextMenuSelectedItemID == ITEM_CHANGE_GNSS_UNITS) changeToScreen(SCREEN_SELECT_GNSS_UNITS);
        if(contextMenuSelectedItemID == ITEM_RESET_AGL_ALTITUDE)
        {
          if(GNSSTelemetryData.positionFix != 0)
            Model.gnssAltitudeOffset = GNSSTelemetryData.altitude;
          else
            Model.gnssAltitudeOffset = Model.gnssLastKnownAltitude;
          //also reset statistics for the Altitude AGL sensor
          for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
          {
            if(Model.Telemetry[i].identifier == SENSOR_ID_GNSS_AGL_ALTITUDE)
            {
              telemetryMaxReceivedValue[i] = TELEMETRY_NO_DATA;
              telemetryMinReceivedValue[i] = TELEMETRY_NO_DATA;
            }
          }
          changeToScreen(SCREEN_TELEMETRY);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_STARTING_POINT)
        {
          Model.gnssHomeLatitude = GNSSTelemetryData.latitude;
          Model.gnssHomeLongitude = GNSSTelemetryData.longitude;
          //also reset statistics for the Distance sensor
          for(uint8_t i = 0; i < NUM_CUSTOM_TELEMETRY; i++)
          {
            if(Model.Telemetry[i].identifier == SENSOR_ID_GNSS_DISTANCE)
            {
              telemetryMaxReceivedValue[i] = TELEMETRY_NO_DATA;
              telemetryMinReceivedValue[i] = TELEMETRY_NO_DATA;
            }
          }
          changeToScreen(SCREEN_TELEMETRY);
        }
        if(contextMenuSelectedItemID == ITEM_RESET_LAST_KNOWN_LOCATION)
        {
          Model.gnssLastKnownLatitude = 0;
          Model.gnssLastKnownLongitude = 0;
          Model.gnssLastKnownAltitude = 0;
          Model.gnssLastKnownDistanceFromHome = 0;
          changeToScreen(SCREEN_TELEMETRY);
        }

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case SCREEN_SENSOR_STATISTICS:
      {
        drawHeader(PSTR("Statistics"));
        
        static uint8_t page = 0;
        static bool viewInitialised = false;
        static uint8_t topItem = 1;
        if(!viewInitialised)
        {
          page = thisTelemIdx;
          focusedItem = 1;
          topItem = 1;
          viewInitialised = true;
        }
        
        //--- dynamic scrollable list
        
        enum {
          ITEM_MINIMUM_VAL,
          ITEM_RESET_MINIMUM_VAL,
          ITEM_MAXIMUM_VAL,
          ITEM_RESET_MAXIMUM_VAL,
          ITEM_LAST_RECEIVED_VAL,
          ITEM_LAST_RECEIVED_TIME,
          ITEM_RESET_LAST_RECEIVED_VAL,
          
          ITEM_COUNT
        };
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        //add items IDs to the list
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(!Model.Telemetry[page].recordMaximum && (i == ITEM_MAXIMUM_VAL || i == ITEM_RESET_MAXIMUM_VAL))
            continue;
          if(!Model.Telemetry[page].recordMinimum && (i == ITEM_MINIMUM_VAL || i == ITEM_RESET_MINIMUM_VAL))
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        uint8_t numFocusable = listItemCount + 1; //+1 for title focus
        changeFocusOnUpDown(numFocusable);
        if(focusedItem == 1) //title focus
          topItem = 1;
        else if(focusedItem > 1)
        {
          if(focusedItem - 1 < topItem)
            topItem = focusedItem - 1;
          while(focusedItem - 1 >= topItem + 5)
            topItem++;
        }
        
        //show title
        display.setCursor(8, 9);
        display.print(Model.Telemetry[page].name);
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        //show current val, right aligned.
        //using a hack to measure the width of the text before printing to actual position on screen
        drawTelemetryValue(0, 64, page, telemetryReceivedValue[page], false);
        uint8_t len_px = display.getCursorX();
        uint8_t xpos = 127 - len_px;
        drawTelemetryValue(xpos, 9, page, telemetryReceivedValue[page], true);
        display.drawRect(xpos - 2, 7, len_px + 3, 11, BLACK);

        //fill list and edit items
        for(uint8_t line = 0; line < 5 && line < listItemCount; line++)
        {
          uint8_t ypos = 20 + line*9;
          if(focusedItem - 1 == topItem + line)
            drawCursor(58, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool isFocused = (focusedItem > 1 && itemID == listItemIDs[focusedItem - 2]);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_MAXIMUM_VAL:
              {
                display.print(F("Maximum:"));
                drawTelemetryValue(66, ypos, page, telemetryMaxReceivedValue[page], false);
              }
              break;
            
            case ITEM_RESET_MAXIMUM_VAL:
              {
                display.setCursor(66, ypos);
                display.print(F("[Reset]"));
                if(isFocused && clickedButton == KEY_SELECT)
                  telemetryMaxReceivedValue[page] = telemetryReceivedValue[page];
              }
              break;
            
            case ITEM_MINIMUM_VAL:
              {
                display.print(F("Minimum:"));
                drawTelemetryValue(66, ypos, page, telemetryMinReceivedValue[page], false);
              }
              break;
              
            case ITEM_RESET_MINIMUM_VAL:
              {
                display.setCursor(66, ypos);
                display.print(F("[Reset]"));
                if(isFocused && clickedButton == KEY_SELECT)
                  telemetryMinReceivedValue[page] = telemetryReceivedValue[page]; 
              }
              break;
              
            case ITEM_LAST_RECEIVED_VAL:
              {
                display.print(F("Last val:"));
                drawTelemetryValue(66, ypos, page, telemetryLastReceivedValue[page], false);
              }
              break;
              
            case ITEM_LAST_RECEIVED_TIME:
              {
                display.print(F("Time rcvd:"));
                display.setCursor(66, ypos);
                if(telemetryLastReceivedValue[page] == TELEMETRY_NO_DATA)
                  display.print(F("No data"));
                else
                {
                  uint32_t elapsedSeconds = (millis() - telemetryLastReceivedTime[page]) / 1000;
                  if(elapsedSeconds >= 3600)
                  {
                    uint8_t hours = elapsedSeconds / 3600;
                    display.print(hours);
                    display.setCursor(display.getCursorX() + 3, display.getCursorY());
                    display.print(F("hr"));
                  }
                  else if(elapsedSeconds >= 60)
                  {
                    uint8_t minutes = elapsedSeconds / 60;
                    display.print(minutes);
                    display.setCursor(display.getCursorX() + 3, display.getCursorY());
                    display.print(F("min"));
                  }
                  else
                  {
                    display.print(elapsedSeconds);
                    display.setCursor(display.getCursorX() + 3, display.getCursorY());
                    display.print(F("s"));
                  }
                  display.print(F(" ago"));
                }
              }
              break;
              
            case ITEM_RESET_LAST_RECEIVED_VAL:
              {
                display.setCursor(66, ypos);
                display.print(F("[Reset]"));
                if(isFocused && clickedButton == KEY_SELECT)
                  telemetryLastReceivedValue[page] = telemetryReceivedValue[page]; 
              }
              break;
          }
        }
        
        //scrollbar
        drawScrollBar(127, 19, listItemCount, topItem, 5, 5 * 9);
        
        //change to next sensor
        if(focusedItem == 1)
        { 
          drawCursor(0, 9);
          toggleEditModeOnSelectClicked();  
          if(isEditMode && (buttonCode == KEY_UP || buttonCode == KEY_DOWN))
          {
            do {
              page = incDec(page, 0, NUM_CUSTOM_TELEMETRY - 1, INCDEC_WRAP, INCDEC_SLOW);
            } while(isEmptyStr(Model.Telemetry[page].name, sizeof(Model.Telemetry[0].name)) //skip empty
                    || Model.Telemetry[page].type == TELEMETRY_TYPE_GNSS); //skip special telemetry
          }
        }
        
        //exit
        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;
          changeToScreen(SCREEN_TELEMETRY);
        }
      }
      break;
    
    case SCREEN_EDIT_SENSOR:
      {
        telemetry_params_t *tlm = &Model.Telemetry[thisTelemIdx]; 
        
        //--- dynamic scrollable list
        
        enum {
          ITEM_TELEMETRY_NAME,
          ITEM_TELEMETRY_UNITSNAME,
          ITEM_TELEMETRY_IDENTIFIER,
          ITEM_TELEMETRY_FACTOR10,
          ITEM_TELEMETRY_MULTIPLIER,
          ITEM_TELEMETRY_OFFSET,
          ITEM_TELEMETRY_ALARM_CONDITION,
          ITEM_TELEMETRY_ALARM_THRESHOLD,
          ITEM_TELEMETRY_SHOW_ON_HOME,
          ITEM_TELEMETRY_RECORD_MAXIMUM,
          ITEM_TELEMETRY_RECORD_MINIMUM,
          
          ITEM_COUNT
        };
        
        static bool isEditingName = false;
        static bool isEditingUnits = false;
        
        if(isEditTextDialog)
        {
          if(isEditingName) 
            editTextDialog(PSTR("Sensor name"), tlm->name, sizeof(tlm->name), false, true, false);
          else if(isEditingUnits)
            editTextDialog(PSTR("Units"),  tlm->unitsName, sizeof(tlm->unitsName), true, true, false);
          break;
        }
        //reset
        isEditingName = false;
        isEditingUnits = false;

        drawHeader(PSTR("Sensor"));
        
        uint8_t listItemIDs[ITEM_COUNT]; 
        uint8_t listItemCount = 0;
        
        //add item Ids to the list of Ids
        for(uint8_t i = 0; i < sizeof(listItemIDs); i++)
        {
          if(i == ITEM_TELEMETRY_ALARM_THRESHOLD && tlm->alarmCondition == TELEMETRY_ALARM_CONDITION_NONE)
            continue;
          listItemIDs[listItemCount++] = i;
        }
        
        //handle navigation
        changeFocusOnUpDown(listItemCount); 
        toggleEditModeOnSelectClicked();
        static uint8_t topItem = 1;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + 7)
          topItem++;
        
        //fill list and edit items
        for(uint8_t line = 0; line < 7 && line < listItemCount; line++)
        {
          uint8_t ypos = 8 + line*8;
          if(focusedItem == topItem + line)
            drawCursor(58, ypos);
          
          if((topItem - 1 + line) >= listItemCount)
            break;
          
          uint8_t itemID = listItemIDs[topItem - 1 + line];
          bool edit = (itemID == listItemIDs[focusedItem - 1] && isEditMode);
          
          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_TELEMETRY_NAME:
              {
                display.print(F("Name:"));
                display.setCursor(66, ypos);
                if(isEmptyStr(tlm->name, sizeof(tlm->name)))
                  display.print(F("--"));
                else
                  display.print(tlm->name);
                if(edit)
                {
                  isEditTextDialog = true;
                  isEditingName = true;
                }
              }
              break;
            
            case ITEM_TELEMETRY_UNITSNAME:
              {
                display.print(F("Units:"));
                display.setCursor(66, ypos);
                if(isEmptyStr(tlm->unitsName, sizeof(tlm->unitsName)))
                  display.print(F("--"));
                else 
                  display.print(tlm->unitsName);
                if(edit)
                {
                  isEditTextDialog = true;
                  isEditingUnits = true;
                }
              }
              break;
            
            case ITEM_TELEMETRY_IDENTIFIER: 
              {
                display.print(F("ID:"));
                display.setCursor(66, ypos);
                display.print(F("0x"));
                if(tlm->identifier < 16)
                  display.print(F("0"));
                display.print(tlm->identifier, HEX);
                if(edit)
                  tlm->identifier = incDec(tlm->identifier, 0x00, 0xFE, INCDEC_NOWRAP, INCDEC_NORMAL);
              }
              break;

            case ITEM_TELEMETRY_MULTIPLIER:
              {
                display.print(F("Multplr:"));
                display.setCursor(66, ypos);
                int16_t val = tlm->multiplier;
                display.print(val / 100);
                display.print(F("."));
                val = val % 100;
                if(val < 10) 
                  display.print(F("0"));
                display.print(val);
                if(edit)
                  tlm->multiplier = incDec(tlm->multiplier, 1, 10000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;
            
            case ITEM_TELEMETRY_FACTOR10:
              {
                display.print(F("Factor10:"));
                display.setCursor(66, ypos);
                display.print(tlm->factor10);
                if(edit)
                  tlm->factor10 = incDec(tlm->factor10, -3, 3, INCDEC_NOWRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_TELEMETRY_OFFSET:
              {
                display.print(F("Offset:"));
                display.setCursor(66, ypos);
                printTelemParam(thisTelemIdx, tlm->offset, false);
                if(edit)
                  tlm->offset = incDec(tlm->offset, -30000, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;

            case ITEM_TELEMETRY_ALARM_CONDITION:
              {
                display.print(F("Alerts:"));
                display.setCursor(66, ypos);
                display.print(findStringInIdStr(enum_TelemetryAlarmCondition, tlm->alarmCondition));
                if(edit)
                  tlm->alarmCondition = incDec(tlm->alarmCondition, 0, TELEMETRY_ALARM_CONDITION_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
              }
              break;
            
            case ITEM_TELEMETRY_ALARM_THRESHOLD:
              {
                display.print(F("Threshold:"));
                display.setCursor(66, ypos);
                printTelemParam(thisTelemIdx, tlm->alarmThreshold, true);
                if(edit)
                  tlm->alarmThreshold = incDec(tlm->alarmThreshold, -30000, 30000, INCDEC_NOWRAP, INCDEC_NORMAL, INCDEC_FAST);
              }
              break;
            
            case ITEM_TELEMETRY_SHOW_ON_HOME:
              {
                display.print(F("On home:"));
                drawCheckbox(66, ypos, tlm->showOnHome);
                if(edit)
                  tlm->showOnHome = incDec(tlm->showOnHome, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_TELEMETRY_RECORD_MAXIMUM:
              {
                display.print(F("RecordMax:"));
                drawCheckbox(66, ypos, tlm->recordMaximum);
                if(edit)
                  tlm->recordMaximum = incDec(tlm->recordMaximum, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
            
            case ITEM_TELEMETRY_RECORD_MINIMUM:
              {
                display.print(F("RecordMin:"));
                drawCheckbox(66, ypos, tlm->recordMinimum);
                if(edit)
                  tlm->recordMinimum = incDec(tlm->recordMinimum, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
              }
              break;
          }
        }
        
        //Draw scroll bar
        drawScrollBar(127, 8, listItemCount, topItem, 7, 7 * 8);
        
        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;
      
    case CONFIRMATION_DELETE_SENSOR:
      {
        printFullScreenMessage(PSTR("Delete sensor?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          resetTelemParams(thisTelemIdx);
          //reset any associated logical switches
          for(uint8_t i = 0; i < NUM_LOGICAL_SWITCHES; i++)
          {
            if(Model.LogicalSwitch[i].func <= LS_FUNC_GROUP3_LAST 
               && Model.LogicalSwitch[i].val1 == (int16_t) SRC_TELEMETRY_FIRST + thisTelemIdx)
            {
              resetLogicalSwitchParams(i);
            }
          }
          //reset any associated widgets
          for(uint8_t i = 0; i < NUM_WIDGETS; i++)
          {
            if(Model.Widget[i].type == WIDGET_TYPE_TELEMETRY && Model.Widget[i].src == thisTelemIdx)
            {
              resetWidgetParams(i);
            }
          }
          //exit
          changeToScreen(SCREEN_TELEMETRY);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_TELEMETRY);
      }
      break;

    case SCREEN_SELECT_GNSS_UNITS:
      {
        drawHeader(PSTR("Select units"));

        display.setCursor(0, 9);
        display.print(F("Distance:"));
        display.setCursor(72, 9);
        display.print(findStringInIdStr(enum_DisplayedUnits, Model.gnssDistanceUnits));
        
        display.setCursor(0, 18);
        display.print(F("Speed:"));
        display.setCursor(72, 18);
        display.print(findStringInIdStr(enum_DisplayedUnits, Model.gnssSpeedUnits));
        
        display.setCursor(0, 27);
        display.print(F("Altitude:"));
        display.setCursor(72, 27);
        display.print(findStringInIdStr(enum_DisplayedUnits, Model.gnssAltitudeUnits));

        drawDottedHLine(0, 54, 128, BLACK, WHITE);
        display.setCursor(78, 56);
        display.print(F("[Finish]"));
        if(focusedItem == 4)
          drawCursor(70, 56);

        if(focusedItem < 4)
          drawCursor(64, focusedItem * 9);
        
        changeFocusOnUpDown(4);
        toggleEditModeOnSelectClicked();
        
        //edit items
        if(focusedItem == 1)
          Model.gnssDistanceUnits = incDec(Model.gnssDistanceUnits, UNITS_DISTANCE_FIRST, UNITS_DISTANCE_LAST, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 2)
          Model.gnssSpeedUnits = incDec(Model.gnssSpeedUnits, UNITS_SPEED_FIRST, UNITS_SPEED_LAST, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 3 && isEditMode && (buttonCode == KEY_UP || buttonCode == KEY_DOWN))
        {
          do {
          Model.gnssAltitudeUnits = incDec(Model.gnssAltitudeUnits, UNITS_DISTANCE_FIRST, UNITS_DISTANCE_LAST, INCDEC_WRAP, INCDEC_SLOW);
          } while (Model.gnssAltitudeUnits != UNITS_METRES && Model.gnssAltitudeUnits != UNITS_FEET );
        }

        //exit
        if(focusedItem == 4 && clickedButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_TELEMETRY);
        }
      }
      break;

    case SCREEN_TELEMETRY_GNSS:
      {
        drawHeader(PSTR("GNSS data"));

        enum {
          ITEM_SATELLITES,
          ITEM_DISTANCE,
          ITEM_SPEED,
          ITEM_COURSE,
          ITEM_AGL_ALTITUDE,
          ITEM_MSL_ALTITUDE,
          ITEM_LATITUDE,
          ITEM_LONGITUDE,
          ITEM_SEPARATOR,
          ITEM_TITLE_HOME_LOCATION,
          ITEM_HOME_LATITUDE,
          ITEM_HOME_LONGITUDE,

          ITEM_COUNT
        };

        //initialise
        static uint8_t thisPage = 1;
        static bool viewInitialised = false;
        if(!viewInitialised)
        {
          thisPage = 1;
          viewInitialised = true;
        }

        //handle navigation
        uint8_t numPages = (ITEM_COUNT + 5) / 6;
        isEditMode = true;
        thisPage = incDec(thisPage, numPages, 1, INCDEC_WRAP, INCDEC_SLOW);

        //fill list
        for(uint8_t line = 0; line < 6 && line < ITEM_COUNT; line++)
        {
          uint8_t ypos = 9 + line * 9;
          uint8_t itemID = (thisPage - 1) * 6 + line;
          if(itemID >= ITEM_COUNT)
            break;

          display.setCursor(0, ypos);
          switch(itemID)
          {
            case ITEM_SATELLITES:
              {
                display.print(F("Satellite:"));
                display.setCursor(60, ypos);
                display.print(GNSSTelemetryData.satellitesInUse);
                display.print(F("/"));
                display.print(GNSSTelemetryData.satellitesInView);
                if(GNSSTelemetryData.positionFix != 0)
                {
                  display.setCursor(108, ypos);
                  display.print(F("Fix"));
                }
              }
              break;

            case ITEM_LATITUDE:
              {
                display.print(F("Latitude:"));
                display.setCursor(60, ypos);
                if(GNSSTelemetryData.positionFix != 0)
                  printFixedPointVal(GNSSTelemetryData.latitude, 5);
                else
                  printFixedPointVal(Model.gnssLastKnownLatitude, 5);
                display.write(0xF8);
              }
              break;

            case ITEM_LONGITUDE:
              {
                display.print(F("Longitude:"));
                display.setCursor(60, ypos);
                if(GNSSTelemetryData.positionFix != 0)
                  printFixedPointVal(GNSSTelemetryData.longitude, 5);
                else
                  printFixedPointVal(Model.gnssLastKnownLongitude, 5);
                display.write(0xF8);
              }
              break;

            case ITEM_TITLE_HOME_LOCATION:
              {
                display.print(F("Starting point"));
              }
              break;
            
            case ITEM_HOME_LATITUDE:
              {
                display.print(F("Latitude:"));
                display.setCursor(60, ypos);
                printFixedPointVal(Model.gnssHomeLatitude, 5);
                display.write(0xF8);
              }
              break;
            
            case ITEM_HOME_LONGITUDE:
              {
                display.print(F("Longitude:"));
                display.setCursor(60, ypos);
                printFixedPointVal(Model.gnssHomeLongitude, 5);
                display.write(0xF8);
              }
              break;

            case ITEM_MSL_ALTITUDE:
            case ITEM_AGL_ALTITUDE:
            case ITEM_DISTANCE:
              {
                int32_t val = 0; // the raw value is in metres.
                uint8_t units = Model.gnssAltitudeUnits;
                if(itemID == ITEM_MSL_ALTITUDE)
                {
                  display.print(F("Altitude:"));
                  if(GNSSTelemetryData.positionFix != 0)
                    val = GNSSTelemetryData.altitude;
                  else
                    val = Model.gnssLastKnownAltitude;
                }
                else if(itemID == ITEM_AGL_ALTITUDE)
                {
                  display.print(F("Altitude:"));
                  if(GNSSTelemetryData.positionFix != 0)
                    val = GNSSTelemetryData.altitude - Model.gnssAltitudeOffset;
                  else
                    val = Model.gnssLastKnownAltitude - Model.gnssAltitudeOffset;
                }
                else if(itemID == ITEM_DISTANCE)
                {
                  display.print(F("Distance:"));
                  if(GNSSTelemetryData.positionFix != 0)
                    val = gnssDistanceFromHome;
                  else 
                    val = Model.gnssLastKnownDistanceFromHome;
                  units = Model.gnssDistanceUnits;
                }

                display.setCursor(60, ypos);
                 
                switch(units)
                {
                  case UNITS_METRES:
                    {
                      display.print(val);
                      display.setCursor(display.getCursorX() + 3, ypos);
                      display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_METRES));
                    }
                    break;

                  case UNITS_METRES_KILOMETRES:
                    {
                      if(val < 1000)
                      {
                        display.print(val);
                        display.setCursor(display.getCursorX() + 3, ypos);
                        display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_METRES));
                      }
                      else
                      {
                        printFixedPointVal(val, 3);
                        display.setCursor(display.getCursorX() + 3, ypos);
                        display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_KILOMETRES));
                      }
                    }
                    break;

                  case UNITS_KILOMETRES:
                    {
                      printFixedPointVal(val, 3);
                      display.setCursor(display.getCursorX() + 3, ypos);
                      display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_KILOMETRES));
                    }
                    break;

                  case UNITS_FEET:
                    {
                      val = (float) val * 3.28;
                      display.print(val);
                      display.setCursor(display.getCursorX() + 3, ypos);
                      display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_FEET));
                    }
                    break;

                  case UNITS_FEET_MILES:
                    {
                      if(val <= 1610)
                      {
                        val = (float) val * 3.28;
                        display.print(val);
                        display.setCursor(display.getCursorX() + 3, ypos);
                        display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_FEET));
                      }
                      else
                      {
                        val = (float) val * 0.621;
                        printFixedPointVal(val, 3);
                        display.setCursor(display.getCursorX() + 3, ypos);
                        display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_MILES));
                      }
                    }
                    break;

                  case UNITS_MILES:
                    {
                      val = (float) val * 0.621;
                      printFixedPointVal(val, 3);
                      display.setCursor(display.getCursorX() + 3, ypos);
                      display.print(findStringInIdStr(enum_DisplayedUnits, UNITS_MILES));
                    }
                    break;
                }

                display.setCursor(108, ypos);
                if(itemID == ITEM_MSL_ALTITUDE)
                  display.print(F("MSL"));
                else if(itemID == ITEM_AGL_ALTITUDE)
                  display.print(F("AGL"));
              }
              break;
            
            case ITEM_SPEED:
              {
                display.print(F("Speed:"));
                display.setCursor(60, ypos);
                int32_t val = GNSSTelemetryData.speed;
                switch(Model.gnssSpeedUnits)
                {
                  case UNITS_METRES_PER_SECOND:
                    printFixedPointVal(val, 1);
                    break;

                  case UNITS_KILOMETRES_PER_HOUR:
                    val = (val * 360) / 100;
                    printFixedPointVal(val, 1);
                    break;

                  case UNITS_FEET_PER_SECOND:
                    val = (val * 328) / 100;
                    printFixedPointVal(val, 1);
                    break;

                  case UNITS_MILES_PER_HOUR:
                    val = (val * 224) / 100;
                    printFixedPointVal(val, 1);
                    break;
                  
                  case UNITS_KNOTS:
                    val = (val * 194) / 100;
                    printFixedPointVal(val, 1);
                    break;
                }
                display.setCursor(display.getCursorX() + 3, ypos);
                display.print(findStringInIdStr(enum_DisplayedUnits, Model.gnssSpeedUnits));
              }
              break;

            case ITEM_COURSE:
              {
                display.print(F("Course:"));
                display.setCursor(60, ypos);
                display.print(GNSSTelemetryData.course / 10);
                display.print(F("."));
                display.print(GNSSTelemetryData.course % 10);
                display.write(0xF8);
              }
              break;
          }
        }

        //Draw scroll bar
        drawScrollBar(127, 9, numPages, thisPage, 1, 1 * 54);

        //exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_TELEMETRY);
          viewInitialised = false;
        }
      }
      break;
  }
}

#endif
