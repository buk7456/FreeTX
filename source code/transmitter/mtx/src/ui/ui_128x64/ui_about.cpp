#include "ui_128x64.h"

#if defined (UI_128X64)

void ui_handler_about()
{
  switch(theScreen)
  {
      case SCREEN_ABOUT:
      {
        drawHeader_Menu(mainMenu[MAIN_MENU_ABOUT]);
        
        static uint8_t topItem = 1, highlightedItem = 1;
        
        enum {
          ITEM_VERSION_INFO,
          ITEM_DISCLAIMER,
          ITEM_THIRD_PARTY_NOTICES,
          ITEM_EASTER_EGG,
        };

        menuInitialise();
        menuAddItem(PSTR("Version info"), ITEM_VERSION_INFO, NULL);
        menuAddItem(PSTR("Disclaimer"), ITEM_DISCLAIMER, NULL);
        menuAddItem(PSTR("Third party notices"), ITEM_THIRD_PARTY_NOTICES, NULL);
        menuAddItem(PSTR(""), ITEM_EASTER_EGG, NULL);
        menuDraw(&topItem, &highlightedItem);

        if(menuSelectedItemID == ITEM_VERSION_INFO)
        {
          textViewerText = versionText;
          changeToScreen(SCREEN_TEXT_VIEWER);
        }
        else if(menuSelectedItemID == ITEM_DISCLAIMER)
        {
          textViewerText = disclaimerText;
          changeToScreen(SCREEN_TEXT_VIEWER);
        }
        else if(menuSelectedItemID == ITEM_THIRD_PARTY_NOTICES)
        {
          textViewerText = thirdPartyNoticesText;
          changeToScreen(SCREEN_TEXT_VIEWER);
        }
        else if(menuSelectedItemID == ITEM_EASTER_EGG)
        {
          changeToScreen(SCREEN_EASTER_EGG);
        }

        //exit
        if(heldButton == KEY_SELECT)
        {
          if(!Sys.rememberMenuPosition)
          {
            topItem = 1;
            highlightedItem = 1;
          }
          changeToScreen(SCREEN_MAIN_MENU);
        }
      }
      break;
      
      case SCREEN_EASTER_EGG:
      {
        //------ Simple flappy bird game -------
        
        display.setInterlace(false);
        
        //note that some values here have been empirically determined
        
        const int16_t BIRD_HEIGHT = 12;
        const int16_t BIRD_LENGTH = 17;
        const int16_t BIRD_YMAX = 50;
        const int16_t PIPE_WIDTH = 17;
        const int16_t PIPE_GAP = 30;
        
        enum {
          PIPE_VARIANT_A,
          PIPE_VARIANT_B,
          PIPE_VARIANT_C,
          PIPE_VARIANT_COUNT
        };
        
        static const uint8_t bgTerrain[128] PROGMEM = {
          0x04, 0x0c, 0x06, 0x02, 0x03, 0x06, 0x04, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x04, 0x0c, 0x08, 
          0x08, 0x0c, 0x06, 0x04, 0x0c, 0x0c, 0x04, 0x04, 0x0c, 0x08, 0x08, 0x0e, 0x03, 0x03, 0x02, 0x02, 
          0x06, 0x04, 0x04, 0x0c, 0x08, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x04, 0x07, 0x01, 0x03, 
          0x02, 0x02, 0x02, 0x02, 0x02, 0x06, 0x04, 0x04, 0x06, 0x02, 0x06, 0x03, 0x07, 0x04, 0x04, 0x04, 
          0x0c, 0x08, 0x0c, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x06, 0x03, 0x07, 0x04, 0x04, 0x04, 
          0x0c, 0x06, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x08, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 
          0x06, 0x03, 0x01, 0x01, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x03, 0x03, 0x06, 0x06, 0x02, 
          0x02, 0x06, 0x04, 0x04, 0x0e, 0x08, 0x08, 0x08, 0x0c, 0x06, 0x06, 0x04, 0x06, 0x02, 0x06, 0x04
        };
        
        enum {
          GAME_STATE_INIT,
          GAME_STATE_PLAYING,
          GAME_STATE_OVER,
          GAME_STATE_PAUSED
        };
        
        static int16_t hiScore;
        
        //Dynamically allocated memory for the game
        
        static struct FlappyGame {
          float    birdX;
          float    birdY; 
          float    pipeX;
          float    pipeY;
          float    bgX;
          uint8_t  pipeVariant;
          int16_t  score;
          bool     hasScored;
          float    gravity; 
          float    verticalVelocity; 
          float    scrollAcc; 
          float    scrollVelocity;
          float    scrollVelocityMax;
          bool     maxScrollVelocityReached;
          float    bgScrollAcc;
          float    bgScrollVelocity;
          uint32_t prevMillis;
          uint8_t  state;
        } *game = NULL;
        
        //Allocate memory if it hasn't been allocated yet
        
        if(game == NULL)
        {
          game = (struct FlappyGame *)malloc(sizeof(struct FlappyGame));
          if(game == NULL) //allocation failed
          {
            showMessage(PSTR("Failed to allocate\nmemory for game"));
            delay(2000);
            //force exit
            changeToScreen(SCREEN_ABOUT);
            break;
          }
          else
            game->state = GAME_STATE_INIT;
        }
        
        //Run the game
        
        switch(game->state)
        {
          case GAME_STATE_INIT:
            {
              printFullScreenMessage(PSTR("Get ready"));
              if(clickedButton == KEY_SELECT || millis() - buttonReleaseTime > 1000)
              {
                //Initialise variables
                game->birdX = 24.0;
                game->birdY = 11.0;
                game->pipeX = 127.0;
                game->pipeY = random(5 + PIPE_GAP, 51);
                game->bgX = 0.0;
                game->pipeVariant = random() % PIPE_VARIANT_COUNT;
                game->score = 0;
                game->hasScored = false;
                game->gravity = 185.0; // pixels per second per second
                game->verticalVelocity = 0.0;
                game->scrollAcc = 1.4; // pixels per second per second
                game->scrollVelocity = 20.0; // pixels per second
                game->scrollVelocityMax = 112.0; // pixels per second
                game->maxScrollVelocityReached = false;
                game->bgScrollAcc = 0.5; // pixels per second per second
                game->bgScrollVelocity = 5.0; // pixels per second
                game->prevMillis = millis();
                //Change state
                game->state = GAME_STATE_PLAYING;
                killButtonEvents();
              }
            }
            break;
          
          case GAME_STATE_PLAYING:
            {
              // Every frame, we perform these two calculations in order:
              // 1. We update the velocity, i.e. velocity = velocity + (acceleration × delta time)
              // 2. We update the position, i.e. position = position + (velocity × delta time)

              //--- Calculate the vertical position of the bird

              // When we press the jump button in a Flappy Bird clone, we instantly override the bird's vertical velocity.
              if(pressedButton == KEY_SELECT || pressedButton == KEY_UP) 
                game->verticalVelocity = -4 * sqrt(game->gravity); // About some constant * sqrt(gravity). Constant determined to be about -4
              
              float deltaTime = (millis() - game->prevMillis) / 1000.0f;
              game->prevMillis = millis();

              game->verticalVelocity = game->verticalVelocity + (game->gravity * deltaTime);
              game->birdY = game->birdY + (game->verticalVelocity * deltaTime);
              
              if(game->birdY > BIRD_YMAX)
                game->birdY = BIRD_YMAX;
              
              //--- Calculate the pipe x position
              
              if(!game->maxScrollVelocityReached)
                game->scrollVelocity = game->scrollVelocity + (game->scrollAcc * deltaTime);
              if(game->scrollVelocity > game->scrollVelocityMax)
              {
                game->scrollVelocity = game->scrollVelocityMax;
                game->maxScrollVelocityReached = true;
              }
              game->pipeX = game->pipeX - (game->scrollVelocity * deltaTime);

              // game->maxScrollVelocityReached = false; //##debug
              
              //--- Generate new pipe when out of view
              if(game->pipeX < 0.0 - PIPE_WIDTH)
              {
                game->pipeX = 127;
                game->pipeVariant = random() % PIPE_VARIANT_COUNT;
                if(game->pipeVariant == PIPE_VARIANT_C)
                  game->pipeY = random(5 + PIPE_GAP, 51);
                if(game->pipeVariant == PIPE_VARIANT_B)
                  game->pipeY = random(5 + PIPE_GAP, 62);
                if(game->pipeVariant == PIPE_VARIANT_A)
                  game->pipeY = random(PIPE_GAP, 51);
              }
              
              //--- Calculate background position.
              //The background is parallax scrolled
              if(!game->maxScrollVelocityReached)
                game->bgScrollVelocity = game->bgScrollVelocity + (game->bgScrollAcc * deltaTime);
              game->bgX = game->bgX + (game->bgScrollVelocity * deltaTime);

              if(game->bgX > 127)
                game->bgX = 0.0;
              
              ///--- Detect collisions ---
              
              bool collided = false; 
              
              //collision of bird with floor
              if(game->birdY >= BIRD_YMAX)
                collided = true;
              
              //collision of bird with pipe
              if(game->pipeX < (game->birdX + BIRD_LENGTH) && game->birdX < (game->pipeX + PIPE_WIDTH))
              {
                if(game->pipeVariant == PIPE_VARIANT_A || game->pipeVariant == PIPE_VARIANT_C)
                {
                  if(game->pipeY < (game->birdY + BIRD_HEIGHT))
                    collided = true;
                }
                if(game->pipeVariant == PIPE_VARIANT_B || game->pipeVariant == PIPE_VARIANT_C)
                {
                  if(game->birdY < (game->pipeY - PIPE_GAP + 1))
                    collided = true;
                }
              }
              
              // collided = false; //###Debug
              
              ///--- Calculate scores
              
              if(game->birdX > (game->pipeX + PIPE_WIDTH) && !game->hasScored && !collided)
              {
                game->hasScored = true;
                game->score++;
                if(game->score > hiScore)
                  hiScore = game->score;
              }
              if(game->birdX < game->pipeX)
                game->hasScored = false;
              
              ///--- Draw on the screen
              
              //draw the background
              uint8_t idx = game->bgX;
              for(uint8_t x = 0; x < 128; x++, idx++)
              {
                if(idx > 127)
                  idx = 0;
                uint8_t val = pgm_read_byte(bgTerrain + idx);
                for(uint8_t b = 0; b < 4; b++)
                {
                  if(val & (1 << b))
                    display.drawPixel(x, 54 + b, BLACK);
                }
              }
              
              //Draw the foreground
              display.drawHLine(0, 62, 128, BLACK);
              int16_t gx = game->pipeX;
              while(gx >= 0)
              {
                display.drawPixel(gx, 63, BLACK);
                display.drawPixel(gx + 1, 63, BLACK);
                gx -= 6;
              }
              gx = game->pipeX;
              while(gx < 128)
              {
                display.drawPixel(gx, 63, BLACK);
                display.drawPixel(gx + 1, 63, BLACK);
                gx += 6;
              }
              
              //Draw the pipe
              if(game->pipeVariant == PIPE_VARIANT_A || game->pipeVariant == PIPE_VARIANT_C)
              {
                //lower pipe
                display.drawRect(game->pipeX, game->pipeY, PIPE_WIDTH, 4, BLACK);
                display.fillRect(game->pipeX + 1, game->pipeY + 3, PIPE_WIDTH - 2, 60 - game->pipeY, WHITE);
                display.drawRect(game->pipeX + 1, game->pipeY + 3, PIPE_WIDTH - 2, 60 - game->pipeY, BLACK);
              }
              if(game->pipeVariant == PIPE_VARIANT_B || game->pipeVariant == PIPE_VARIANT_C)
              {
                //upper pipe
                display.drawRect(game->pipeX, game->pipeY - PIPE_GAP - 3, PIPE_WIDTH, 4, BLACK);
                display.drawVLine(game->pipeX + 1, 0, game->pipeY - PIPE_GAP - 3, BLACK);
                display.drawVLine(game->pipeX + PIPE_WIDTH - 2, 0, game->pipeY - PIPE_GAP - 3, BLACK);
              }
              
              //Draw the bird
              display.fillRect(game->birdX, game->birdY, BIRD_LENGTH, BIRD_HEIGHT, WHITE);
              drawAnimatedSprite(game->birdX, game->birdY, animation_bird, BIRD_LENGTH, BIRD_HEIGHT, BLACK, 3, 80, 0, true);

              //Draw the score
              display.setCursor(0, 0);
              display.print(game->score);
              
              //###Debug draw
              // display.setCursor(0, 8);
              // display.print(game->scrollVelocity);
              // display.setCursor(0, 16);
              // display.print(game->bgScrollVelocity);
              
              ///--- Play audio
              /* 
              if(audioToPlay = AUDIO_KEY_PRESSED) //always suppress key press tones
                audioToPlay = AUDIO_NONE;
              if(game->hasScored)
                audioToPlay = AUDIO_NONE; //replace with actual
              if(collided)
                audioToPlay = AUDIO_NONE; //replace with actual 
              */

              ///--- Change game state
              
              //change state if collided
              if(collided)
              {
                game->state = GAME_STATE_OVER;
                killButtonEvents();
              }
            }
            break;
          
          case GAME_STATE_OVER:
            {
              drawBoundingBox(11, 14, 105, 35,BLACK);
              display.setCursor(17, 18);
              display.print(F("Game over"));
              display.setCursor(17, 28);
              display.print(F("Score:   "));
              display.print(game->score);
              display.setCursor(17, 38);
              display.print(F("HiScore: "));
              display.print(hiScore);
              
              if(clickedButton == KEY_SELECT)
              {
                game->state = GAME_STATE_INIT;
                killButtonEvents();
              }
            }
            break;
        }
        
        //Exit the game, freeing the allocated memory
        if(heldButton == KEY_SELECT)
        {
          free(game); //free the memory
          game = NULL; //set the pointer to NULL to avoid dangling pointer issues
          changeToScreen(SCREEN_ABOUT);
        }
      }
      break;
  }
}

#endif
