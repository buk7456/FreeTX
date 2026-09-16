#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_custom_curves()
{
  switch(theScreen)
  {
    case SCREEN_CUSTOM_CURVES:
      {
        drawHeader(extrasMenu[EXTRAS_MENU_CUSTOM_CURVES]);
        
        custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx]; 
        
        display.setCursor(8, 9);
        display.print(F("Crv"));
        display.print(thisCrvIdx + 1);
        if(!isEmptyStr(crv->name, sizeof(crv->name)))
        {
          display.setCursor(display.getCursorX() + 6, 9);
          display.print(crv->name);
        }
        display.drawHLine(8, 17, display.getCursorX() - 9, BLACK);
        
        display.setCursor(0, 20);
        display.print(F("Type:"));
        display.setCursor(38, 20);
        display.print(crv->numPoints);
        display.setCursor(display.getCursorX() + 3, 20);
        display.print(F("pts"));
        
        display.setCursor(0, 29);
        display.print(F("Pt:"));
        display.setCursor(38, 29);
        display.write(97 + thisCrvPt);
        
        display.setCursor(0, 38);
        display.print(F("Xval:"));
        display.setCursor(38, 38);
        display.print(crv->xVal[thisCrvPt]);
        
        display.setCursor(0, 47);
        display.print(F("Yval:"));
        display.setCursor(38, 47);
        display.print(crv->yVal[thisCrvPt]);
        
        display.setCursor(0, 56);
        display.print(F("Smth:"));
        drawCheckbox(38, 56, crv->smooth);

        //draw cursor
        if(focusedItem == 1) 
          drawCursor(0, 9);
        else if(focusedItem < 7)
          drawCursor(30, (focusedItem * 9) + 2);
        
        //draw context menu icon
        display.fillRect(120, 0, 8, 7, WHITE);
        display.drawBitmap(120, 0, focusedItem == 7 ? icon_context_menu_focused : icon_context_menu, 8, 7, BLACK);
        
        //show moved input on graph
        static uint8_t movedSrc = SRC_NONE;
        bool moved = false;
        uint8_t tmpMovedSrc = getMovedSource();
        if(screenshotSwtch != CTRL_SW_NONE && tmpMovedSrc >= SRC_SW_PHYSICAL_FIRST && tmpMovedSrc <= SRC_SW_PHYSICAL_LAST)
        {
          //suppress if it is the switch associated with screenshots
          if(tmpMovedSrc == (SRC_SW_PHYSICAL_FIRST + ((screenshotSwtch - CTRL_SW_PHYSICAL_FIRST) / 6)))
            tmpMovedSrc = SRC_NONE;
        }
        if(tmpMovedSrc != SRC_NONE)
          movedSrc = tmpMovedSrc;
        if(movedSrc != SRC_NONE)
        {
          static int8_t lastVal = 0;
          static uint32_t lastMovedTime = 0;
          int16_t difference = mixSources[movedSrc]/5 - lastVal;
          if(difference >= 5 || difference <= -5)
          {
            lastVal = mixSources[movedSrc]/5;
            lastMovedTime = millis();
          }
          if(millis() - lastMovedTime < 3000)
          {
            moved = true;
            display.setInterlace(false);
          }
          else
            movedSrc = SRC_NONE;
        }

        //draw graph
        drawCustomCurve(crv, (focusedItem >= 3 && focusedItem < 6) ? thisCrvPt : 0xff, moved ? movedSrc : (uint8_t)SRC_NONE);

        //change focus
        changeFocusOnUpDown(7);
        toggleEditModeOnSelectClicked();
        
        //---- edit parameters
        if(focusedItem == 1) //change to next or previous curve
        {
          uint8_t _prevCrvIdx = thisCrvIdx;
          thisCrvIdx = incDec(thisCrvIdx, 0, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
          if(thisCrvIdx != _prevCrvIdx)
            thisCrvPt = 0;
        }
        else if(focusedItem == 2) //change number of points
        {
          uint8_t prevNumPoints = crv->numPoints;
          crv->numPoints = incDec(crv->numPoints, MIN_NUM_POINTS_CUSTOM_CURVE, MAX_NUM_POINTS_CUSTOM_CURVE, 
                                          INCDEC_NOWRAP, INCDEC_SLOW);
          if(crv->numPoints != prevNumPoints) //recalculate points if changed
          {
            calcNewCurvePts(crv, prevNumPoints);
            thisCrvPt = 0;
          }
        }
        else if(focusedItem == 3) //move to next/prev point
          thisCrvPt = incDec(thisCrvPt, 0, crv->numPoints - 1, INCDEC_WRAP, INCDEC_SLOW);
        else if(focusedItem == 4 && isEditMode) //edit x values
        {
          if(thisCrvPt > 0 && thisCrvPt < crv->numPoints - 1) //only edit inner x values
          {
            int8_t _minVal = crv->xVal[thisCrvPt - 1];
            int8_t _maxVal = crv->xVal[thisCrvPt + 1];
            crv->xVal[thisCrvPt] = incDec(crv->xVal[thisCrvPt], _minVal, _maxVal, INCDEC_NOWRAP, INCDEC_NORMAL);
          }
        }
        else if(focusedItem == 5) //edit Y values
          crv->yVal[thisCrvPt] = incDec(crv->yVal[thisCrvPt], -100, 100, INCDEC_NOWRAP, INCDEC_NORMAL);
        else if(focusedItem == 6) //smoothing
          crv->smooth = incDec(crv->smooth, 0, 1, INCDEC_WRAP, INCDEC_PRESSED);
        else if(focusedItem == 7 && isEditMode) //context menu
          changeToScreen(CONTEXT_MENU_CUSTOM_CURVE);

        ////// Exit
        if(heldButton == KEY_SELECT)
        {
          changeToScreen(SCREEN_EXTRAS_MENU);
          thisCrvPt = 0;
        }
      }
      break;

    case CONTEXT_MENU_CUSTOM_CURVE:
      {
        enum {
          ITEM_COPY_CURVE,
          ITEM_RENAME_CURVE,
          ITEM_RESET_CURVE,
          ITEM_FLIP_HORIZONTAL,
          ITEM_FLIP_VERTICAL,
          ITEM_INSERT_POINT,
          ITEM_DELETE_POINT
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Copy curve to"), ITEM_COPY_CURVE);
        contextMenuAddItem(PSTR("Rename curve"), ITEM_RENAME_CURVE);
        contextMenuAddItem(PSTR("Reset curve"), ITEM_RESET_CURVE);
        contextMenuAddItem(PSTR("Flip horizontal"), ITEM_FLIP_HORIZONTAL);
        contextMenuAddItem(PSTR("Flip vertical"), ITEM_FLIP_VERTICAL);
        contextMenuAddItem(PSTR("Insert point"), ITEM_INSERT_POINT);
        contextMenuAddItem(PSTR("Delete point"), ITEM_DELETE_POINT);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_COPY_CURVE)
        {
          destCrvIdx = thisCrvIdx;
          changeToScreen(DIALOG_COPY_CUSTOM_CURVE);
        }
        if(contextMenuSelectedItemID == ITEM_RENAME_CURVE)
          changeToScreen(DIALOG_RENAME_CUSTOM_CURVE);
        if(contextMenuSelectedItemID == ITEM_RESET_CURVE)
        {
          resetCustomCurveParams(thisCrvIdx);
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(contextMenuSelectedItemID == ITEM_FLIP_HORIZONTAL)
        {
          //Reflect about y axis.
          //If say a point M(h,k), then M'(-h,k). This however affects the order of the points so 
          //we have to counter this by reversing the arrays.
          custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
          for(uint8_t i = 0; i < crv->numPoints; i++)
            crv->xVal[i] = 0 - crv->xVal[i];
          //reverse the arrays
          uint8_t start = 0;
          uint8_t end = crv->numPoints - 1;
          while(start < end)
          {
            int8_t tmp = crv->xVal[start];
            crv->xVal[start] = crv->xVal[end];
            crv->xVal[end] = tmp;
            tmp = crv->yVal[start];
            crv->yVal[start] = crv->yVal[end];
            crv->yVal[end] = tmp;
            start++;
            end--;
          }
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(contextMenuSelectedItemID == ITEM_FLIP_VERTICAL)
        {
          //Reflect about x axis.
          //If say a point M(h,k), then M'(h,-k)
          custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
          for(uint8_t i = 0; i < crv->numPoints; i++)
            crv->yVal[i] = 0 - crv->yVal[i];
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(contextMenuSelectedItemID == ITEM_INSERT_POINT)
        {
          if(Model.CustomCurve[thisCrvIdx].numPoints == MAX_NUM_POINTS_CUSTOM_CURVE)
          {
            makeToast(PSTR("Maximum pts reached"), 2000, 0);
            changeToScreen(SCREEN_CUSTOM_CURVES);
          }
          else 
            changeToScreen(DIALOG_INSERT_CURVE_POINT);
        }
        if(contextMenuSelectedItemID == ITEM_DELETE_POINT)
        {
          if(Model.CustomCurve[thisCrvIdx].numPoints == MIN_NUM_POINTS_CUSTOM_CURVE)
          {
            makeToast(PSTR("Minimum pts reached"), 2000, 0);
            changeToScreen(SCREEN_CUSTOM_CURVES);
          }
          else 
            changeToScreen(DIALOG_DELETE_CURVE_POINT);
        }
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_CUSTOM_CURVES);
      }
      break;
      
    case DIALOG_RENAME_CUSTOM_CURVE:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Curve name"), Model.CustomCurve[thisCrvIdx].name, sizeof(Model.CustomCurve[0].name), 
                       true, true, false);
        if(!isEditTextDialog) //exited
          changeToScreen(SCREEN_CUSTOM_CURVES);
      }
      break;
      
    case DIALOG_COPY_CUSTOM_CURVE:
      {
        isEditMode = true;
        destCrvIdx = incDec(destCrvIdx, 0, NUM_CUSTOM_CURVES - 1, INCDEC_WRAP, INCDEC_SLOW);
        drawDialogCopyMove(PSTR("Curve"), thisCrvIdx, destCrvIdx, true);
        if(clickedButton == KEY_SELECT)
        {
          Model.CustomCurve[destCrvIdx] = Model.CustomCurve[thisCrvIdx]; //copy struct
          thisCrvIdx = destCrvIdx;
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_CUSTOM_CURVES);
      }
      break;
      
    case DIALOG_INSERT_CURVE_POINT:
      {
        custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
        
        static bool initialised = false;
        static uint8_t insertionPt = 1; //Point b
        if(!initialised)
        {
          initialised = true;
          if(thisCrvPt >= 1 && thisCrvPt <= crv->numPoints - 1)
            insertionPt = thisCrvPt;  
          else
            insertionPt = 1; 
        }
        
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Insert point"));
        display.setCursor(17, 28);
        display.print(F("before:"));
        display.setCursor(71, 28);
        display.write(97 + insertionPt);
        drawCursor(63, 28);
        
        isEditMode = true;
        insertionPt = incDec(insertionPt, 1, crv->numPoints - 1, INCDEC_WRAP, INCDEC_SLOW);
        
        if(clickedButton == KEY_SELECT)
        {
          //calculate x and y values for the new point
          //For non-smooth curve, it is simply the midpoint. If the curve has smoothing enabled,
          //use cubic interpolation to get the new value for y, as long as the xVal are different.
          int8_t xNew = ((int16_t)crv->xVal[insertionPt - 1] + crv->xVal[insertionPt]) / 2;
          int8_t yNew = ((int16_t)crv->yVal[insertionPt - 1] + crv->yVal[insertionPt]) / 2;
          if(crv->smooth && (crv->xVal[insertionPt - 1] != crv->xVal[insertionPt]))
          {
            int16_t xQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
            int16_t yQQ[MAX_NUM_POINTS_CUSTOM_CURVE];
            for(uint8_t pt = 0; pt < crv->numPoints; pt++)
            {
              xQQ[pt] = 5 * crv->xVal[pt];
              yQQ[pt] = 5 * crv->yVal[pt];
            }
            yNew = cubicHermiteInterpolate(xQQ, yQQ, crv->numPoints, xNew * 5) / 5;
          }
          //shift array elements right and write the new value at the insertionPt
          uint8_t i = MAX_NUM_POINTS_CUSTOM_CURVE - 1;
          while(i > insertionPt)
          {
            crv->xVal[i] = crv->xVal[i-1];
            crv->yVal[i] = crv->yVal[i-1];
            i--;
          }
          crv->xVal[insertionPt] = xNew;
          crv->yVal[insertionPt] = yNew;
          //update point count
          crv->numPoints += 1;
          //exit
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        
        if(heldButton == KEY_SELECT) //exit
        {
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
        }
      }
      break;
      
    case DIALOG_DELETE_CURVE_POINT:
      {
        custom_curve_t *crv = &Model.CustomCurve[thisCrvIdx];
        
        static bool initialised = false;
        static uint8_t deletePt = 1; //Point b
        if(!initialised)
        {
          initialised = true;
          if(thisCrvPt >= 1 && thisCrvPt <= crv->numPoints - 2)
            deletePt = thisCrvPt; 
          else
            deletePt = 1;
        }

        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Delete point"));
        display.setCursor(17, 28);
        display.print(F("Point:"));
        display.setCursor(71, 28);
        display.write(97 + deletePt);
        drawCursor(63, 28);
        
        //Only allow 'deleting' of interior points. We don't technically delete but simply
        //move the element to the end and decrement numPoints by one.
        
        isEditMode = true;
        deletePt = incDec(deletePt, 1, crv->numPoints - 2, INCDEC_WRAP, INCDEC_SLOW);
        
        if(clickedButton == KEY_SELECT)
        {
          //shift all elements that are after the deletionIdx to the left of the deletion Idx
          for(uint8_t i = deletePt; i < MAX_NUM_POINTS_CUSTOM_CURVE - 1; i++) 
          {
            crv->xVal[i] = crv->xVal[i+1];
            crv->yVal[i] = crv->yVal[i+1];
          }
          //update point count
          crv->numPoints -= 1;
          //exit
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
          thisCrvPt = 0;
        }
        
        if(heldButton == KEY_SELECT) //exit
        {
          initialised = false;
          changeToScreen(SCREEN_CUSTOM_CURVES);
        }
      }
      break;
  }
}

#endif
