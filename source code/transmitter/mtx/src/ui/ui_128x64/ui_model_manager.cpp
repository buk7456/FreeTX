#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_model_manager()
{
  switch(theScreen)
  {
    case SCREEN_UNLOCK_MODEL_MANAGER:
      {
        validatePassword(SCREEN_MODEL, SCREEN_MAIN_MENU);
      }
      break;
      
    case SCREEN_MODEL:
      {
        drawHeader(mainMenu[MAIN_MENU_MODEL]);
        
        //------ scrollable list of models ----------
        
        static uint8_t topItem = 1;
        static bool viewInitialised = false;
        static bool modelListNeedsUpdate = true;
        if(!viewInitialised)
        {
          viewInitialised = true;
          modelListNeedsUpdate = true;
          thisModelIdx = Sys.activeModelIdx;
        }
        
        // handle navigation
        focusedItem = thisModelIdx + 1;
        changeFocusOnUpDown(maxNumOfModels);
        uint8_t prevTopItem = topItem;
        if(focusedItem < topItem)
          topItem = focusedItem;
        while(focusedItem >= topItem + VISIBLE_MODELS_IN_MODEL_SCREEN)
          topItem++;
        if(topItem != prevTopItem)
          modelListNeedsUpdate = true;

        // fill list
        //Check at once and only update on scroll or just entered this screen so we 
        //don't have to constantly read all names from EEPROM (a somewhat slow process)
        if(modelListNeedsUpdate)
        {
          modelListNeedsUpdate = false;
          if(maxNumOfModels > 1)
          {
            for(uint8_t i = 0; i < VISIBLE_MODELS_IN_MODEL_SCREEN && i < maxNumOfModels; i++)
            {
              uint8_t modelIdx = topItem - 1 + i;
              if(eeModelIsFree(modelIdx)) // write 0xFF to indicate it is free
                mySharedUnion.modelNameStr[i][0] = 0xFF;
              else //get the name
              {
                eeGetModelName(textBuff, modelIdx, sizeof(textBuff));
                strlcpy(&mySharedUnion.modelNameStr[i][0], textBuff, sizeof(Model.name));
              }
            }
          }
          else
          {
            strlcpy(&mySharedUnion.modelNameStr[0][0], Model.name, sizeof(Model.name));
          }
        }
        
        for(uint8_t line = 0; line < VISIBLE_MODELS_IN_MODEL_SCREEN && line < maxNumOfModels; line++)
        {
          uint8_t ypos = 10 + line * 9;
          uint8_t item = topItem + line;
          if(focusedItem == topItem + line) //highlight
          {
            display.fillRect(0, ypos - 1, item < 10 ? 7 : 13, 9, BLACK);
            display.setTextColor(WHITE);
          }
          display.setCursor(1, ypos);
          display.print(item);
          display.setTextColor(BLACK);
          
          uint8_t modelIdx = item - 1;
          display.setCursor(16, ypos);
          if((uint8_t)mySharedUnion.modelNameStr[line][0] != 0xFF)
            printModelName(mySharedUnion.modelNameStr[line], modelIdx);
          if(modelIdx == Sys.activeModelIdx) //indicate it is active
            display.drawBitmap(display.getCursorX() + 6, ypos + 1, icon_checkmark_bold, 7, 6, BLACK);
        }
        
        //scroll bar
        drawScrollBar(127, 9, maxNumOfModels, topItem, VISIBLE_MODELS_IN_MODEL_SCREEN, VISIBLE_MODELS_IN_MODEL_SCREEN * 9);

        //----- end of list ----------------------
        
        if(focusedItem <= maxNumOfModels)
          thisModelIdx = focusedItem - 1;
        
        if(clickedButton == KEY_SELECT)
        {
          if(thisModelIdx == Sys.activeModelIdx)
            changeToScreen(CONTEXT_MENU_ACTIVE_MODEL);
          else if(!eeModelIsFree(thisModelIdx))
            changeToScreen(CONTEXT_MENU_INACTIVE_MODEL);
          else if(eeModelIsFree(thisModelIdx))
            changeToScreen(CONTEXT_MENU_FREE_MODEL);
          //force the list to be updated
          modelListNeedsUpdate = true; 
        }

        if(heldButton == KEY_SELECT)
        {
          viewInitialised = false;//reset view
          changeToScreen(SCREEN_MAIN_MENU);
        }
      }
      break;
      
    case CONTEXT_MENU_ACTIVE_MODEL:
      {
        enum {
          ITEM_RENAME_MODEL,
          ITEM_DUPLICATE_MODEL,
          ITEM_COPY_FROM,
          ITEM_RESET_MODEL,
          ITEM_BACKUP_MODEL,
          ITEM_RESTORE_MODEL,
          ITEM_NEW_MODEL,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Rename model"), ITEM_RENAME_MODEL);
        if(maxNumOfModels > 1)
        {
          contextMenuAddItem(PSTR("Duplicate"), ITEM_DUPLICATE_MODEL);
          contextMenuAddItem(PSTR("Copy from"), ITEM_COPY_FROM);
        }
        contextMenuAddItem(PSTR("Reset model"), ITEM_RESET_MODEL);
        if(maxNumOfModels == 1)
          contextMenuAddItem(PSTR("New model"), ITEM_NEW_MODEL);
        if(sdHasCard())
        {
          contextMenuAddItem(PSTR("Back up to SD card"), ITEM_BACKUP_MODEL);
          contextMenuAddItem(PSTR("Restore from SD card"), ITEM_RESTORE_MODEL);
        }
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_RENAME_MODEL) 
          changeToScreen(DIALOG_RENAME_MODEL);
        if(contextMenuSelectedItemID == ITEM_DUPLICATE_MODEL)
        {          
          //Find the nearest free slot and copy to it
          uint8_t nearestLeft = 0xff;
          uint8_t nearestRight = 0xff;
          for(uint8_t i = thisModelIdx + 1; i < maxNumOfModels; i++) //search in right direction
          {
            if(eeModelIsFree(i)) 
            {
              nearestRight = i; 
              break;
            }
          }
          for(int8_t i = thisModelIdx - 1; i >= 0; i--) //search in left direction
          {
            if(eeModelIsFree(i))
            {
              nearestLeft = i; 
              break;
            }
          }
          if(nearestLeft == 0xff && nearestRight == 0xff) //no free slots
          {
            makeToast(PSTR("No free slots"), 2000, 0);
            changeToScreen(SCREEN_MODEL);
          }
          else
          {
            uint8_t destination;
            uint8_t distRight = (nearestRight == 0xff) ? 0xff : nearestRight - thisModelIdx;
            uint8_t distLeft = (nearestLeft == 0xff) ? 0xff : thisModelIdx - nearestLeft;
            if(distRight == 0xff)
              destination = nearestLeft;
            else if(distLeft == 0xff)
              destination = nearestRight;
            else
            {
              destination = (distRight <= distLeft) ? nearestRight : nearestLeft;
            }
            //--- copy
            showWaitMessage();
            stopTones();
            //save the active model
            eeSaveModelData(Sys.activeModelIdx);
            //save to this slot and set it active
            eeSaveModelData(destination);
            Sys.activeModelIdx = destination;
            thisModelIdx = destination;
            //exit
            changeToScreen(SCREEN_MODEL);
          }
        }
        if(contextMenuSelectedItemID == ITEM_COPY_FROM)    
          changeToScreen(DIALOG_COPYFROM_MODEL);
        if(contextMenuSelectedItemID == ITEM_RESET_MODEL)  
          changeToScreen(CONFIRMATION_MODEL_RESET);
        if(contextMenuSelectedItemID == ITEM_BACKUP_MODEL)
          changeToScreen(CONFIRMATION_MODEL_BACKUP);
        if(contextMenuSelectedItemID == ITEM_RESTORE_MODEL)
          changeToScreen(DIALOG_RESTORE_MODEL);
        if(contextMenuSelectedItemID == ITEM_NEW_MODEL)
          changeToScreen(CONFIRMATION_CREATE_MODEL);
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONTEXT_MENU_INACTIVE_MODEL:
      {
        enum {
          ITEM_LOAD_MODEL,
          ITEM_DELETE_MODEL,
          ITEM_BACKUP_MODEL,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("Load model"), ITEM_LOAD_MODEL);
        contextMenuAddItem(PSTR("Delete model"), ITEM_DELETE_MODEL);
        if(sdHasCard())
          contextMenuAddItem(PSTR("Back up to SD card"), ITEM_BACKUP_MODEL);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_LOAD_MODEL) 
        {
          showWaitMessage();
          stopTones();
          //Save the active model before changing to another model
          eeSaveModelData(Sys.activeModelIdx);
          //load model and set it active
          eeReadModelData(thisModelIdx);
          Sys.activeModelIdx = thisModelIdx; 
          //Safety checks
          handleSafetyWarningUI();
          //reinitialise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          restoreTimerRegisters();
          restoreCounterRegisters();
          //exit
          changeToScreen(SCREEN_MODEL);
          resetPages();
        }
        if(contextMenuSelectedItemID == ITEM_DELETE_MODEL)
          changeToScreen(CONFIRMATION_MODEL_DELETE);
        if(contextMenuSelectedItemID == ITEM_BACKUP_MODEL)
          changeToScreen(CONFIRMATION_MODEL_BACKUP);
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONTEXT_MENU_FREE_MODEL:
      {
        enum {
          ITEM_NEW_MODEL,
          ITEM_RESTORE_MODEL,
        };
        
        contextMenuInitialise();
        contextMenuAddItem(PSTR("New model"), ITEM_NEW_MODEL);
        if(sdHasCard())
          contextMenuAddItem(PSTR("Restore from SD card"), ITEM_RESTORE_MODEL);
        contextMenuDraw();
        
        if(contextMenuSelectedItemID == ITEM_NEW_MODEL)
          changeToScreen(DIALOG_MODEL_TYPE);
        if(contextMenuSelectedItemID == ITEM_RESTORE_MODEL)
          changeToScreen(DIALOG_RESTORE_MODEL);

        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONFIRMATION_CREATE_MODEL:
      {
        printFullScreenMessage(PSTR("Model data will\nbe overwritten.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
          changeToScreen(DIALOG_MODEL_TYPE);
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case DIALOG_MODEL_TYPE:
      {
        isEditMode = true;
        
        static uint8_t mdlType;
        static bool initialised = false;
        if(!initialised)
        {
          initialised = true;
          mdlType = Model.type;
        }
        
        mdlType = incDec(mdlType, 0, MODEL_TYPE_COUNT - 1, INCDEC_WRAP, INCDEC_SLOW);

        drawBoundingBox(11, 14, 105, 35,BLACK);
        drawCursor(21, 28);
        display.setCursor(17, 18);
        display.print(F("Model type"));
        display.setCursor(29, 28);
        display.print(findStringInIdStr(enum_ModelType, mdlType));
        
        if(clickedButton == KEY_SELECT) //create the model
        {
          showWaitMessage();
          stopTones();
          //Save the current active model first
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          //reinitialise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          //create model and set it active
          if(maxNumOfModels > 1)
          {
            eeCreateModel(thisModelIdx);
            Sys.activeModelIdx = thisModelIdx;
          }
          else
          {
            resetModelName();
            resetModelParams();
          }
          //write model type
          Model.type = mdlType;
          //load default mixer template
          if(mdlType == MODEL_TYPE_AIRPLANE || mdlType == MODEL_TYPE_MULTICOPTER)
          {
            if(Sys.mixerTemplatesEnabled)
              loadMixerTemplateBasic(0);
          }
          //save
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          // changeToScreen(SCREEN_MODEL);
          changeToScreen(DIALOG_RENAME_MODEL);
          resetPages();
        }
        
        if(heldButton == KEY_SELECT) //exit
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case DIALOG_RENAME_MODEL:
      {
        isEditTextDialog = true;
        editTextDialog(PSTR("Model name"), Model.name, sizeof(Model.name), true, true, false);
        if(!isEditTextDialog) //exited
        {
          showWaitMessage();
          stopTones();
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          changeToScreen(SCREEN_MODEL); 
        }
      }
      break;
      
    case DIALOG_COPYFROM_MODEL:
      {
        //change source model
        isEditMode = true;
        do {
          thisModelIdx = incDec(thisModelIdx, 0, maxNumOfModels - 1 , INCDEC_WRAP, INCDEC_SLOW);
        } while(eeModelIsFree(thisModelIdx) || Model.type != eeGetModelType(thisModelIdx));
        
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Copy from"));
        display.setCursor(29, 28);
        eeGetModelName(textBuff, thisModelIdx, sizeof(textBuff)); 
        printModelName(textBuff, thisModelIdx);
        drawCursor(21, 28);

        if(clickedButton == KEY_SELECT)
        {
          if(thisModelIdx == Sys.activeModelIdx)
            changeToScreen(SCREEN_MODEL);
          else
            changeToScreen(CONFIRMATION_MODEL_COPY);
        }
        else if(heldButton == KEY_SELECT) //exit
        {
          thisModelIdx = Sys.activeModelIdx;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case DIALOG_RESTORE_MODEL:
      {
        static bool initialised = false;
        static uint16_t mdlCount = 0;
        static uint16_t thisPos = 0;
        
        if(!initialised)
        {
          initialised = true;
          //show wait message as getting the count may take some time when there are many models
          showWaitMessage();
          stopTones();
          //find how many models we have backed up on the SD card
          mdlCount = sdGetModelCount();
          thisPos = 0;
          sdGetModelName(mySharedUnion.nameStr, thisPos, sizeof(mySharedUnion.nameStr));
        }
        
        if(mdlCount == 0) //no backups exist on the SD card
        {
          makeToast(PSTR("No models on SD card"), 2000, 0);
          initialised = false;
          changeToScreen(SCREEN_MODEL);
          break;
        }
          
        drawBoundingBox(11, 14, 105, 35,BLACK);
        display.setCursor(17, 18);
        display.print(F("Restore from SD"));
        display.setCursor(29, 28);
        display.print(mySharedUnion.nameStr);
        drawCursor(21, 28);
        
        //get the name
        isEditMode = true;
        uint8_t lastPos = thisPos;
        thisPos = incDec(thisPos, 0, mdlCount - 1, INCDEC_NOWRAP, INCDEC_SLOW); //strictly No Wrap here
        if(thisPos != lastPos) //changed, refresh
          sdGetModelName(mySharedUnion.nameStr, thisPos, sizeof(mySharedUnion.nameStr));
        
        if(clickedButton == KEY_SELECT)
        {
          initialised = false;
          changeToScreen(CONFIRMATION_MODEL_RESTORE);
        }

        if(heldButton == KEY_SELECT) //exit
        {
          initialised = false;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case CONFIRMATION_MODEL_RESTORE:
      {
        if(thisModelIdx == Sys.activeModelIdx)
          printFullScreenMessage(PSTR("Model data will be\noverwritten.\nContinue?\n\nYes [Up] \nNo [Down]"));
        
        if(clickedButton == KEY_UP || thisModelIdx != Sys.activeModelIdx)
        {
          showWaitMessage();
          stopTones();
          
          //save active model first but only if we aren't restoring to active slot
          //otherwise there is no point in saving first
          if(maxNumOfModels > 1 && thisModelIdx != Sys.activeModelIdx)
            eeSaveModelData(Sys.activeModelIdx);
          
          //restore the model
          if(sdRestoreModel(mySharedUnion.nameStr))
          {
            //save it to EEPROM
            if(maxNumOfModels > 1)
              eeSaveModelData(thisModelIdx);
            
            //read back the active model from EEPROM if we are not restoring into the active slot
            if(maxNumOfModels > 1 && thisModelIdx != Sys.activeModelIdx)
              eeReadModelData(Sys.activeModelIdx);
            
            //show message if errors were encountered in the imported data
            if(dbgTotalErrorLines > 0)
              handleDataImportWarningUI();
            
            //if we have restored into the active model slot, reinitialise some items
            if(thisModelIdx == Sys.activeModelIdx)
            {
              //safety checks
              handleSafetyWarningUI();
              //reinitialise other stuff
              resetTimerRegisters();
              resetCounterRegisters();
              Sys.rfEnabled = false;
              reinitialiseMixerCalculations();
              restoreTimerRegisters();
              restoreCounterRegisters();
              resetPages();
            }

            //exit
            changeToScreen(SCREEN_MODEL);
          }
          else
          {
            //exit
            makeToast(PSTR("Restore failed"), 2000, 0);
            changeToScreen(SCREEN_MODEL);
          }
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONFIRMATION_MODEL_BACKUP:
      {
        static bool initialised = false;
        static bool modelBackupExists = false;
        
        if(!initialised)
        {
          initialised = true;
          modelBackupExists = false;

          //get the model name into the nameStr buffer
          if(maxNumOfModels > 1)
            eeGetModelName(mySharedUnion.nameStr, thisModelIdx, sizeof(mySharedUnion.nameStr));
          else
            strlcpy(mySharedUnion.nameStr, Model.name, sizeof(mySharedUnion.nameStr));
          
          bool nameEmpty = false;
          if(isEmptyStr(mySharedUnion.nameStr, sizeof(mySharedUnion.nameStr))) 
            nameEmpty = true;
          else
          {
            // Force conforming to 8.3 naming rules.
            char baseName[9];
            strlcpy(baseName, mySharedUnion.nameStr, sizeof(baseName));
            sanitize83BaseName(baseName);
            // copy back to nameStr
            strlcpy(mySharedUnion.nameStr, baseName, sizeof(mySharedUnion.nameStr));
            // check again if empty
            if(mySharedUnion.nameStr[0] == '\0') 
              nameEmpty = true;
          }
          
          if(nameEmpty) //make a generic name
          {
            strlcpy_P(mySharedUnion.nameStr, PSTR("MODEL"), sizeof(mySharedUnion.nameStr));
            char suffix[5];
            memset(suffix, 0, sizeof(suffix));
            itoa(thisModelIdx + 1, suffix, 10);
            strlcat(mySharedUnion.nameStr, suffix, sizeof(mySharedUnion.nameStr));
          }
          
          //append file extension
          // strlcat(mySharedUnion.nameStr, ".MDL", sizeof(mySharedUnion.nameStr)); //is this really necessary??
          
          //show wait message as checking may take some time when there are many models
          showWaitMessage();
          stopTones();
          
          //check if the name already exists
          if(sdSimilarModelExists(mySharedUnion.nameStr))
            modelBackupExists = true;
        }
        
        if(modelBackupExists)
          printFullScreenMessage(PSTR("A backup with\na similar name\nalready exists.\nOverwrite it?\n\nYes [Up] \nNo [Down]"));
        
        if(clickedButton == KEY_UP || !modelBackupExists)
        {
          showWaitMessage();
          stopTones();
          
          if(maxNumOfModels > 1)
          {
            //save the active model first
            eeSaveModelData(Sys.activeModelIdx);
            //read into ram the model we want to back up to SD card
            eeReadModelData(thisModelIdx);
          }
          
          //back up to the SD card
          if(!sdBackupModel(mySharedUnion.nameStr))
            makeToast(PSTR("Back up failed"), 2000, 0);
          
          if(maxNumOfModels > 1)
          {
            //read back the active model from EEPROM
            eeReadModelData(Sys.activeModelIdx);
          }

          //exit
          initialised = false;
          changeToScreen(SCREEN_MODEL);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
        {
          initialised = false;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case CONFIRMATION_MODEL_COPY:
      {
        printFullScreenMessage(PSTR("Model data will\nbe overwritten.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          showWaitMessage();
          stopTones();
          //temporarily store model name as we shall maintain it 
          strlcpy(textBuff, Model.name, sizeof(textBuff));
          //load source model into ram
          eeReadModelData(thisModelIdx);
          //restore the model name
          strlcpy(Model.name, textBuff, sizeof(Model.name));
          //save
          eeSaveModelData(Sys.activeModelIdx);
          //reinitialise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          thisModelIdx = Sys.activeModelIdx; 
          //exit
          changeToScreen(SCREEN_MODEL);
          resetPages();
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
        {
          thisModelIdx = Sys.activeModelIdx;
          changeToScreen(SCREEN_MODEL);
        }
      }
      break;
      
    case CONFIRMATION_MODEL_RESET:
      {
        printFullScreenMessage(PSTR("Model data will\nbe reset.\nContinue?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          showWaitMessage();
          stopTones();
          //temporarily store the model type for later restoring
          uint8_t mdlType = Model.type;
          //reset
          resetModelParams();
          //load the default mixer template
          if(mdlType == MODEL_TYPE_AIRPLANE || mdlType == MODEL_TYPE_MULTICOPTER)
          {
            if(Sys.mixerTemplatesEnabled)
              loadMixerTemplateBasic(0);
          }
          //restore the model type
          Model.type = mdlType;
          //save to EEPROM
          if(maxNumOfModels > 1)
            eeSaveModelData(Sys.activeModelIdx);
          //reinitialise other stuff
          resetTimerRegisters();
          resetCounterRegisters();
          Sys.rfEnabled = false;
          reinitialiseMixerCalculations();
          //exit
          changeToScreen(SCREEN_MODEL);
          resetPages();
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
      
    case CONFIRMATION_MODEL_DELETE:
      {
        printFullScreenMessage(PSTR("Delete model?\n\nYes [Up] \nNo [Down]"));
        if(clickedButton == KEY_UP)
        {
          showWaitMessage();
          stopTones();
          eeDeleteModel(thisModelIdx);
          changeToScreen(SCREEN_MODEL);
        }
        else if(clickedButton == KEY_DOWN || heldButton == KEY_SELECT)
          changeToScreen(SCREEN_MODEL);
      }
      break;
  }
}

#endif
  
