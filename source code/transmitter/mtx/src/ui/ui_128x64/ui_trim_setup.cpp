#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_trim_setup()
{
  switch(theScreen)
  {
    case SCREEN_TRIM_SETUP:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_TRIM_SETUP]);
        
        uint8_t qqSrc[4] = {SRC_X1_TRIM, SRC_Y1_TRIM, SRC_X2_TRIM, SRC_Y2_TRIM};

        trim_params_t* qqTrim[4];
        qqTrim[0] = &Model.X1Trim;
        qqTrim[1] = &Model.Y1Trim;
        qqTrim[2] = &Model.X2Trim;
        qqTrim[3] = &Model.Y2Trim;

        int16_t fmdTrimVal[4];
        fmdTrimVal[0] = Model.FlightMode[activeFmdIdx].x1Trim;
        fmdTrimVal[1] = Model.FlightMode[activeFmdIdx].y1Trim;
        fmdTrimVal[2] = Model.FlightMode[activeFmdIdx].x2Trim;
        fmdTrimVal[3] = Model.FlightMode[activeFmdIdx].y2Trim;
        
        for(uint8_t i = 0; i < 4; i++)
        {
          display.setCursor(0, 9 + i * 9);
          getSrcName(textBuff, qqSrc[i], sizeof(textBuff));
          display.print(textBuff);
          display.print(F(":"));
          display.setCursor(42, 9 + i * 9);
          display.print(findStringInIdStr(enum_TrimState, qqTrim[i]->trimState));
          display.setCursor(84, 9 + i * 9);
          display.print(F("("));
          int16_t val = 0;
          if(qqTrim[i]->trimState == TRIM_COMMON) 
            val = qqTrim[i]->commonTrim;
          else if(qqTrim[i]->trimState == TRIM_FLIGHT_MODE) 
            val = fmdTrimVal[i];
          if(val < 0)
          {
            display.print(F("-"));
            val = -val;
          }
          display.print(val / 10);
          display.print(F("."));
          display.print(val % 10);
          display.print(F(")"));
        }
        
        display.setCursor(0, 45);
        display.print(F("Step:"));
        display.setCursor(42, 45);
        display.print(findStringInIdStr(enum_TrimStep, Model.trimStep));

        changeFocusOnUpDown(5);
        toggleEditModeOnSelectClicked();
        drawCursor(34, focusedItem * 9);
        
        //edit
        if(isEditMode && focusedItem <= 4 && (buttonCode == KEY_UP || buttonCode == KEY_DOWN))
        {
          uint8_t i = focusedItem - 1;
          do {
            qqTrim[i]->trimState = incDec(qqTrim[i]->trimState, 0, TRIM_STATE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);
          } while(Model.type == MODEL_TYPE_OTHER && qqTrim[i]->trimState == TRIM_FLIGHT_MODE);
        }
        if(focusedItem == 5)
          Model.trimStep = incDec(Model.trimStep, 0, TRIM_STEP_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);

        //exit
        if(heldButton == KEY_SELECT)
          changeToScreen(SCREEN_EXTRAS_MENU);
      }
      break;
  }
}

#endif
