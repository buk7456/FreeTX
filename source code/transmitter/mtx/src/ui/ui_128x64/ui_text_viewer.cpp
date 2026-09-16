#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_text_viewer()
{
  switch(theScreen)
  {
    case SCREEN_TEXT_VIEWER:
      {
        static uint16_t startPos; //the offset into the text

        static uint16_t scrollOffsetQQ[6]; //buffer to help with scrolling. Stores offsets into the text.
        static uint8_t idxQQ = 0;
        
        static bool initialised = false;
        if(!initialised)
        {
          initialised = true;
          startPos = 0;
          idxQQ = 0;
        }
        
        uint16_t pos = startPos; //position in text
        bool isEnd = false;
        
        //Print the text with both line wrap and word wrap. Word wrap prioritized. 
        //Doesn't cater for all edge cases, but works good enough.
        uint8_t line = 0, i = 0;
        bool isPageBreak = false;
        while(line < 6 && !isEnd)
        {
          while(i < 21)
          {
            char c = pgm_read_byte(textViewerText + pos);
            //Check for end of text
            if(c == '\0')
            {
              isEnd = true;
              break;
            }
            //Skip carriage return, or if we are at start of line and the character is a space
            if(c == '\r' || (i == 0 && c == ' '))
            {
              pos++; //advance position in text
              continue;
            }
            //Check if it is a new line character
            if(c == '\n')
            {
              pos++; //advance position in text
              break;
            }
            //check if it is a form feed character
            if(c == '\f')
            {
              isPageBreak = true;
              pos++; //advance position in text
              break;
            }
            
            //Figure out how long the word is. If it is longer than the space left on the 
            //line, force it onto a new line. If the word is longer than the total space on the line, 
            //just print anyway but starting on a new line. 
            if(pgm_read_byte(textViewerText + pos - 1) == ' ' || pos == 0) //start of word, prev character is a space
            {
              uint8_t wordLen = 0;
              uint8_t j = 0;
              char wordCharacter;
              while((wordCharacter = pgm_read_byte(textViewerText + pos + j)) != ' ')
              {
                if(wordCharacter == '\0' || wordCharacter == '\r' || wordCharacter == '\n' || wordCharacter == '\f')
                  break;
                wordLen++;
                j++;
              }
              uint8_t remainingSpace = 20 - i;
              if(wordLen > remainingSpace && i != 0)
                break;
            }
            //Write the character to the screen
            display.setCursor(1 + i*6, 6 + line*9);
            display.write(c);
            pos++; //advance position in text
            i++; //advance position in line
          }
          
          line++; //advance line
          i = 0; //reset position in line
          
          if(isPageBreak)
            break;
        }
        
        //Show arrow icons
        if(!isEnd)
          display.drawBitmap(61, 61, icon_down_arrow_small, 5, 3, BLACK);
        if(startPos > 0)
          display.drawBitmap(61, 0, icon_up_arrow_small, 5, 3, BLACK);
        
        //Handle scrolling down
        if(pressedButton == KEY_DOWN && !isEnd)
        {
          scrollOffsetQQ[idxQQ] = startPos; //store the current offset
          idxQQ++;
          if(idxQQ >= sizeof(scrollOffsetQQ)/scrollOffsetQQ[0])
          {
            idxQQ = sizeof(scrollOffsetQQ)/scrollOffsetQQ[0] - 1;
            //In this case, we have reached the buffer's limit, therefore 
            //remove the oldest offset by shifting elements in the array to the left.
            for(uint8_t i = 0; i < sizeof(scrollOffsetQQ)/scrollOffsetQQ[0] - 1; i++)
            {
              scrollOffsetQQ[i] = scrollOffsetQQ[i + 1];
            }
          }
          //update the start position
          startPos = pos;
        }
        //Handle scrolling up
        if(pressedButton == KEY_UP && startPos > 0)
        {
          if(idxQQ > 0)
          {
            startPos = scrollOffsetQQ[idxQQ - 1];
            idxQQ--;
          }
          else
          {
            //Here, we have reached the buffer's limit and the workaround is to recalculate the startPos
            //by simply subtracting off the number of characters that can be fitted on a single screen.
            if(startPos >= 126) // 21*6
              startPos -= 126;
            else
              startPos = 0;
          }
        }
        
        //Exit
        if(heldButton == KEY_SELECT)
        {
          initialised = false;
          changeToScreen(lastScreen);
        }
      }
      break;
  }
}

#endif
