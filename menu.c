
/*
 *
 * menu.c
 * Created on the 23rd of August 2005
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Deals with the loading, running and freeing of the menus.
 *
 */


#include "player.h"
#include <string.h>
#include <time.h>


SDL_Surface *menuScreens[15];
SDL_Color    menuPalettes[4][256];
int          episodes;


int loadMenu () {

  FILE *f;
  time_t currentTime;
  int count, end, colour;

  f = fopen("openjazz.000", "rb");

  if (f == NULL) return FAILURE;

  menuScreens[14] = loadSurface(f, 64, 40);

  fclose(f);

  if (loadMusic("menusng.psm")) {

    SDL_FreeSurface(menuScreens[14]);

    return FAILURE;

  }


  f = fopenFromPath("menu.000");

  if (f == NULL) {

    freeMusic();
    SDL_FreeSurface(menuScreens[14]);

    return FAILURE;

  }

  fseek(f, 0, SEEK_END);
  end = ftell(f);
  fseek(f, 0, SEEK_SET);

  // Load the main menu graphics
  loadPalette(menuPalettes[0], f);
  menuScreens[0] = loadSurface(f, 320, 200);
  menuScreens[1] = loadSurface(f, 320, 200);


  // In December, load the Christmas menu graphics
  if (end > 200000) {

    time(&currentTime);

    if (localtime(&currentTime)->tm_mon == 11) {

      SDL_FreeSurface(menuScreens[0]);
      SDL_FreeSurface(menuScreens[1]);
      loadPalette(menuPalettes[0], f);
      menuScreens[0] = loadSurface(f, 320, 200);
      menuScreens[1] = loadSurface(f, 320, 200);

    } else {

      skipRLE(f);
      skipRLE(f);
      skipRLE(f);

    }

  }

  SDL_SetColorKey(menuScreens[0], SDL_SRCCOLORKEY, 0);
  SDL_SetColorKey(menuScreens[1], SDL_SRCCOLORKEY, 0);

  // Load the difficulty graphics
  loadPalette(menuPalettes[1], f);
  menuScreens[2] = loadSurface(f, 320, 200);
  SDL_SetColorKey(menuScreens[2], SDL_SRCCOLORKEY, 0);

  // Load the episode pictures (max. 10 episodes + bonus level)

  // Load their palette
  loadPalette(menuPalettes[2], f);

  // Generate a greyscale mapping
  for (count = 0; count < 256; count++) {

    colour = ((menuPalettes[2][count].r >> 1) +
              (menuPalettes[2][count].g << 1) +
              (menuPalettes[2][count].b >> 1)  ) / 8;

    if (colour > 79) colour = 79;

    menuPalettes[3][count].r = menuPalettes[3][count].g =
    menuPalettes[3][count].b = colour;

  }

  for (count = 0; count < 11; count++) {

    menuScreens[count + 3] = loadSurface(f, 134, 110);

    if (ftell(f) >= end) {

      episodes = ++count;

      for (; count < 11; count++)
        menuScreens[count + 3] = createBlankSurface();

      fclose(f);

      return SUCCESS;

    }

  }

  // This should never be reached

  episodes = count;

  fclose(f);

  return SUCCESS;

}


void freeMenu (void) {

  int count;

  freeMusic();

  for (count = 0; count < 14; count++) SDL_FreeSurface(menuScreens[count]);

  return;

}


int textInputMenuLoop (char * request, char ** text) {

  char *input;
  SDL_Event event;
  SDL_Rect dst;
  int count, cursor, terminate;

  input = malloc(sizeof(char) * (STRING_LENGTH + 1));
  if (*text != NULL) strcpy(input, *text);
  else input[0] = 0;

  cursor = strlen(input);

  while (1) {

    update();


    // Process system events
    while (SDL_PollEvent(&event)) {

      switch (event.type) {

        case SDL_KEYDOWN:

          // Ensure there is space for another character
          if (cursor < STRING_LENGTH) {

            terminate = (input[cursor] == 0);

            if ((event.key.keysym.sym == ' ') ||
                ((event.key.keysym.sym >= '0') &&
                 (event.key.keysym.sym <= '9')) ||
                ((event.key.keysym.sym >= 'A') &&
                 (event.key.keysym.sym <= 'Z'))) {

              input[cursor] = event.key.keysym.sym;
              cursor++;
              if (terminate != 0) input[cursor] = 0;

            } else if ((event.key.keysym.sym >= 'a') &&
                       (event.key.keysym.sym <= 'z')) {

              input[cursor] = event.key.keysym.sym - 32;
              cursor++;
              if (terminate != 0) input[cursor] = 0;

            }

          }

          if ((event.key.keysym.sym == SDLK_DELETE) &&
              (cursor < strlen(input))) {

            for (count = cursor; count < STRING_LENGTH; count++)
              input[count] = input[count + 1];

          }

          if ((event.key.keysym.sym == SDLK_BACKSPACE) && (cursor > 0)) {

              for (count = cursor - 1; count < STRING_LENGTH; count++)
                input[count] = input[count + 1];

              cursor--;

          }

          // The absence of a break statement is intentional

        case SDL_KEYUP:

          for (count = 0; count < CONTROLS; count++)
            if (event.key.keysym.sym == keys[count].key)
              keys[count].state = event.key.state;

          break;

        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:

          for (count = 0; count < CONTROLS; count++)
            if (event.jbutton.button == buttons[count].button)
              buttons[count].state = event.jbutton.state;

          break;

        case SDL_JOYAXISMOTION:

          for (count = 0; count < CONTROLS; count++)
            if (event.jaxis.axis == axes[count].axis) {

              if (!axes[count].direction && (event.jaxis.value < -16384))
                axes[count].state = SDL_PRESSED;

              else if (axes[count].direction && (event.jaxis.value > 16384))
                axes[count].state = SDL_PRESSED;

              else axes[count].state = SDL_RELEASED;

            }

        break;

#ifndef FULLSCREEN_ONLY
        case SDL_VIDEORESIZE:

          screenW = event.resize.w;
          screenH = event.resize.h;
          if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
          else bgScale = ((screenH - 34) / 100) + 1;
          screen = SDL_SetVideoMode(screenW, screenH, 8,
                                    SDL_RESIZABLE | SDL_DOUBLEBUF |
                                    SDL_HWSURFACE | SDL_HWPALETTE  );

          // The absence of a break statement is intentional

        case SDL_VIDEOEXPOSE:

          SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
          SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

          break;
#endif

        case SDL_QUIT:

          return QUIT;

      }

    }

    // Apply controls to universal control tracking
    for (count = 0; count < CONTROLS; count++) {

      if ((keys[count].state == SDL_PRESSED) ||
          (buttons[count].state == SDL_PRESSED) || 
          (axes[count].state == SDL_PRESSED)      ) {

        if (controls[count].time < SDL_GetTicks())
          controls[count].state = SDL_PRESSED;

      } else {

        controls[count].time = 0;
        controls[count].state = SDL_RELEASED;

      }

    }


    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 15);

    // Draw the prompt
    showString(request, 32, (screenH / 2) - 16, fontmn2);

    // Draw the section of the text before the cursor
    scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);
    terminate = input[cursor];
    input[cursor] = 0;
    dst.x = showString(input, 40, screenH / 2, fontmn2);

    // Draw the cursor
    dst.w = 8;
    dst.h = 2;
    dst.y = (screenH / 2) + 10;
    SDL_FillRect(screen, &dst, 79);

    // Draw the section of text after the cursor
    input[cursor] = terminate;
    showString(input + cursor, dst.x, screenH / 2, fontmn2);
    restorePalette(fontmn2->pixels);


    if (controls[C_LEFT].state == SDL_PRESSED) {

      releaseControl(C_LEFT);

      if (cursor > 0) cursor--;

    }

    if (controls[C_RIGHT].state == SDL_PRESSED) {

      releaseControl(C_RIGHT);

      if (cursor < strlen(input)) cursor++;

    }

    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      *text = input;

      return SUCCESS;

    }

  }

  return SUCCESS;

}


int newGameDifficultyMenuLoop (void) {

  char *options[4] = {"EASY", "MEDIUM", "HARD", "TURBO"};
  SDL_Rect src, dst;
  int count;

  usePalette(menuPalettes[1]);

  while (1) {

    if (loop() == QUIT) return QUIT;

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 0);

    for (count = 0; count < 4; count++) {

      if (count == difficulty)
        scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

      showString(options[count], screenW >> 2,
                 (screenH >> 1) + (count << 4) - 32, fontmn2);

      if (count == difficulty)
        restorePalette(fontmn2->pixels);

    }

    src.x = (difficulty & 1) * 160;
    src.y = (difficulty & 2) * 50;
    src.w = 160;
    src.h = 100;
    dst.x = (screenW >> 1) - 40;
    dst.y = (screenH >> 1) - 50;
    SDL_BlitSurface(menuScreens[2], &src, screen, &dst);

    if (controls[C_UP].state == SDL_PRESSED) {

      releaseControl(C_UP);
      difficulty = (difficulty + 3) % 4;

    }

    if (controls[C_DOWN].state == SDL_PRESSED) {

      releaseControl(C_DOWN);
      difficulty = (difficulty + 1) % 4;

    }

    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      freeMusic();

      // Create the player
      nPlayers = 1;
      players = malloc(sizeof(player) * nPlayers);
      localPlayer = players;

      // Set the player's defaults
      localPlayer->name = localPlayerName;
      localPlayer->score = 0;
      localPlayer->lives = 3;
      localPlayer->ammoType = -1;
      localPlayer->ammo[0] = 0;
      localPlayer->ammo[1] = 0;
      localPlayer->ammo[2] = 0;
      localPlayer->ammo[3] = 0;
      localPlayer->fireSpeed = 0;

      // Play the level(s)
      if (runLevel(nextLevel) == QUIT) {

        free(players);
        localPlayer = NULL;

        return QUIT;

      }

      free(players);
      localPlayer = NULL;

      loadMusic("menusng.psm");

      return SUCCESS;

    }

  }

  return SUCCESS;

}


int newGameLevelMenuLoop (void) {

  int option;

  world = level = option = 0;

  usePalette(menuPalettes[1]);

  while (1) {

    if (loop()) return QUIT;

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 15);

    if (option == 0) scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);
    showString("CHOOSE WORLD:", 32, screenH / 3, fontmn2);
    showNumber(world, 208, screenH / 3, fontmn2);

    if (option == 0) restorePalette(fontmn2->pixels);
    else scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

    showString("CHOOSE LEVEL:", 32, (screenH << 1) / 3, fontmn2);
    showNumber(level, 208, (screenH << 1) / 3, fontmn2);

    if (option != 0) restorePalette(fontmn2->pixels);

    if (controls[C_UP].state == SDL_PRESSED) {

      releaseControl(C_UP);

      option ^= 1;

    }

    if (controls[C_DOWN].state == SDL_PRESSED) {

      releaseControl(C_DOWN);

      option ^= 1;

    }

    if (controls[C_LEFT].state == SDL_PRESSED) {

      releaseControl(C_LEFT);

      if (option) level = (level + 9) % 10;
      else world = (world + 999) % 1000;

    }

    if (controls[C_RIGHT].state == SDL_PRESSED) {

      releaseControl(C_RIGHT);

      if (option) level = (level + 1) % 10;
      else world = (world + 1) % 1000;

    }

    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      free(nextLevel);
      nextLevel = malloc(11);
      sprintf(nextLevel, "level%1i.%03i", level, world);

      if (newGameDifficultyMenuLoop() == QUIT) return QUIT;

      usePalette(menuPalettes[1]);

    }

  }

  return SUCCESS;

}


int newGameMenuLoop (void) {

  char *options[12] = {"EPISODE 1",
                       "EPISODE 2",
                       "EPISODE 3",
                       "EPISODE 4",
                       "EPISODE 5",
                       "EPISODE 6",
                       "EPISODE A",
                       "EPISODE B",
                       "EPISODE C",
                       "EPISODE X",
                       "BONUS STAGE",
                       "SPECIFIC LEVEL"};
  int exists[12];
  char check[11];
  SDL_Rect dst;
  int episode, count;

  usePalette(menuPalettes[2]);

  for (count = 0; count < 10; count++) {

    if (count < 6) world = count * 3;
    else if ((count >= 6) && (count < 9)) world = (count + 4) * 3;
    else world = 50;

    sprintf(check, "level0.%03i", world);

    exists[count] = fileExists(check);

    if (exists[count]) restorePalette(menuScreens[count + 3]);
    else SDL_SetPalette(menuScreens[count + 3], SDL_LOGPAL, menuPalettes[3], 0,
                        256);

  }

  exists[10] = 0;
  exists[11] = 1;

  episode = 0;

  while (1) {

    if (loop() == QUIT) return QUIT;

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 0);

    if ((episode < episodes - 1) || (episode < 6)) {

      dst.x = screenW - 150;
      dst.y = (screenH - 110) >> 1;
      SDL_BlitSurface(menuScreens[episode + 3], NULL, screen, &dst);

    } else if ((episode == 10) && (episodes > 6)) {

      dst.x = screenW - 160;
      dst.y = (screenH - 110) >> 1;
      SDL_BlitSurface(menuScreens[episodes + 2], NULL, screen, &dst);

    } else if (episode == 11) {

      // To do

    }

    for (count = 0; count < 12; count++) {

      if (count == episode) {

        scalePalette(fontmn2->pixels, -F10, (-240 * -10) + 79);
        dst.x = (screenW >> 3) - 4;
        dst.y = (screenH >> 1) + (count << 4) - 94;
        dst.w = 136;
        dst.h = 15;
        SDL_FillRect(screen, &dst, 79);

      } else if (!exists[count])
        scalePalette(fontmn2->pixels, -F2, (-240 * -2) + 94);

      showString(options[count], screenW >> 3,
                 (screenH >> 1) + (count << 4) - 92, fontmn2);

      if ((count == episode) || (!exists[count]))
        scalePalette(fontmn2->pixels, F10, (-240 * 10) + 9);

    }

    if (controls[C_UP].state == SDL_PRESSED) {

      releaseControl(C_UP);

      episode = (episode + 11) % 12;

    }

    if (controls[C_DOWN].state == SDL_PRESSED) {

      releaseControl(C_DOWN);

      episode = (episode + 1) % 12;

    }

    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      if (exists[episode]) {

        if (episode < 10) {

          if (episode < 6) world = episode * 3;
          else if ((episode >= 6) && (episode < 9)) world = (episode + 4) * 3;
          else world = 50;
          level = 0;

          free(nextLevel);
          nextLevel = malloc(11);
          sprintf(nextLevel, "level0.%03i", world);

          if (newGameDifficultyMenuLoop() == QUIT) return QUIT;

        } else if (episode == 10) {

          // To do

        } else {

          if (newGameLevelMenuLoop() == QUIT) return QUIT;

        }

        usePalette(menuPalettes[2]);

      }

    }

  }

  return SUCCESS;

}


int loadGameMenuLoop (void) {

  // To do

  return SUCCESS;

}


int setupCharacterMenuLoop (void) {

  char *options[1] = {"NAME"};
  char *result;
  int option, count;

  option = 0;

  while (1) {

    if (loop() == QUIT) return QUIT;

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 0);

    for (count = 0; count < 1; count++) {

      if (count == option)
        scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

      showString(options[count], screenW >> 2,
                 (screenH >> 1) + (count << 4) - 32, fontmn2);

      if (count == option)
        restorePalette(fontmn2->pixels);

    }

    if (controls[C_UP].state == SDL_PRESSED) {

      releaseControl(C_UP);

      option = (option + 1) % 1;

    }

    if (controls[C_DOWN].state == SDL_PRESSED) {

      releaseControl(C_DOWN);

      option = (option + 1) % 1;

    }

    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      switch (option) {

        case 0:

          result = localPlayerName;
          textInputMenuLoop("CHARACTER NAME:", &result);

          if (result != localPlayerName) {

            free(localPlayerName);
            localPlayerName = result;

          }

          break;

      }

    }

  }

  return SUCCESS;

}


int setupKeyboardMenuLoop (void) {

  char *options[7] = {"UP",
                      "DOWN",
                      "LEFT",
                      "RIGHT",
                      "JUMP",
                      "FIRE",
                      "WEAPON"};
  SDL_Event event;
  int progress, count, used;

  progress = 0;

  while (1) {

    update();


    // Process system events
    while (SDL_PollEvent(&event)) {

      switch (event.type) {

        case SDL_KEYDOWN:
        case SDL_KEYUP:

          used = 0;

          for (count = 0; count < CONTROLS; count++)
            if (event.key.keysym.sym == keys[count].key) {

              keys[count].state = event.key.state;
              if (count != progress) used = 1;

            }

          if (!used) {

            keys[progress].key = event.key.keysym.sym;
            progress++;

            if (progress == 7) return SUCCESS;

          }

          break;

#ifndef FULLSCREEN_ONLY
        case SDL_VIDEORESIZE:

          screenW = event.resize.w;
          screenH = event.resize.h;
          if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
          else bgScale = ((screenH - 34) / 100) + 1;
          screen = SDL_SetVideoMode(screenW, screenH, 8,
                                    SDL_RESIZABLE | SDL_DOUBLEBUF |
                                    SDL_HWSURFACE | SDL_HWPALETTE  );

          // The absence of a break statement is intentional

        case SDL_VIDEOEXPOSE:

          SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
          SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

          break;
#endif

        case SDL_QUIT:

          return QUIT;

      }

    }

    // Apply controls to universal control tracking
    for (count = 0; count < CONTROLS; count++) {

      if ((keys[count].state == SDL_PRESSED) ||
          (buttons[count].state == SDL_PRESSED) || 
          (axes[count].state == SDL_PRESSED)      ) {

        if (controls[count].time < SDL_GetTicks())
          controls[count].state = SDL_PRESSED;

      } else {

        controls[count].time = 0;
        controls[count].state = SDL_RELEASED;

      }

    }


    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 0);

    for (count = 0; count < 7; count++) {

      if (count < progress)
        showString("OKAY", (screenW >> 2) + 176,
                   (screenH >> 1) + (count << 4) - 32, fontmn2);

      else if (count == progress)
        scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

      showString(options[count], screenW >> 2,
                 (screenH >> 1) + (count << 4) - 32, fontmn2);

      if (count == progress) {

        showString("PRESS KEY", (screenW >> 2) + 112,
                   (screenH >> 1) + (count << 4) - 32, fontmn2);

        restorePalette(fontmn2->pixels);

      }

    }

  }

  return SUCCESS;

}


int setupJoystickMenuLoop (void) {

  char *options[7] = {"UP",
                      "DOWN",
                      "LEFT",
                      "RIGHT",
                      "JUMP",
                      "FIRE",
                      "WEAPON"};
  SDL_Event event;
  int progress, count, used;

  progress = 0;

  while (1) {

    update();


    // Process system events
    while (SDL_PollEvent(&event)) {

      switch (event.type) {

        case SDL_KEYDOWN:
        case SDL_KEYUP:

          for (count = 0; count < CONTROLS; count++)
            if (event.key.keysym.sym == keys[count].key)
              keys[count].state = event.key.state;

          break;

        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:

          used = 0;

          for (count = 0; count < CONTROLS; count++)
            if (event.jbutton.button == buttons[count].button) {
              buttons[count].state = event.jbutton.state;
              if (count != progress) used = 1;

            }

          if (!used) {

            buttons[progress].button = event.jbutton.button;
            progress++;

            if (progress == 7) return SUCCESS;

          }

          break;

        case SDL_JOYAXISMOTION:

          used = 0;

          for (count = 0; count < CONTROLS; count++)
            if (event.jaxis.axis == axes[count].axis) {

              if (!axes[count].direction && (event.jaxis.value < -16384)) {

                axes[count].state = SDL_PRESSED;
                if (count != progress) used = 1;

              } else if (axes[count].direction && (event.jaxis.value > 16384)) {

                axes[count].state = SDL_PRESSED;
                if (count != progress) used = 1;

              } else axes[count].state = SDL_RELEASED;

            }

          if (!used &&
              ((event.jaxis.value < -16384) || (event.jaxis.value > 16384))) {

            axes[progress].axis = event.jaxis.axis;
            if (event.jaxis.value < -16384) axes[progress].direction = 0;
            else axes[progress].direction = 1;
            progress++;

            if (progress == 7) return SUCCESS;

          }

          break;

#ifndef FULLSCREEN_ONLY
        case SDL_VIDEORESIZE:

          screenW = event.resize.w;
          screenH = event.resize.h;
          if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
          else bgScale = ((screenH - 34) / 100) + 1;
          screen = SDL_SetVideoMode(screenW, screenH, 8,
                                    SDL_RESIZABLE | SDL_DOUBLEBUF |
                                    SDL_HWSURFACE | SDL_HWPALETTE  );

          // The absence of a break statement is intentional

        case SDL_VIDEOEXPOSE:

          SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
          SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

          break;
#endif

        case SDL_QUIT:

          return QUIT;

      }

    }

    // Apply controls to universal control tracking
    for (count = 0; count < CONTROLS; count++) {

      if ((keys[count].state == SDL_PRESSED) ||
          (buttons[count].state == SDL_PRESSED) || 
          (axes[count].state == SDL_PRESSED)      ) {

        if (controls[count].time < SDL_GetTicks())
          controls[count].state = SDL_PRESSED;

      } else {

        controls[count].time = 0;
        controls[count].state = SDL_RELEASED;

      }

    }


    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 0);

    for (count = 0; count < 7; count++) {

      if (count < progress)
        showString("OKAY", (screenW >> 2) + 176,
                   (screenH >> 1) + (count << 4) - 32, fontmn2);

      else if (count == progress)
        scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

      showString(options[count], screenW >> 2,
                 (screenH >> 1) + (count << 4) - 32, fontmn2);

      if (count == progress) {

        showString("PRESS CONTROL", (screenW >> 2) + 112,
                   (screenH >> 1) + (count << 4) - 32, fontmn2);

        restorePalette(fontmn2->pixels);

      }

    }

  }

  return SUCCESS;

}


int setupResolutionMenuLoop (void) {

#ifndef FULLSCREEN_ONLY

  int widthOptions[] = {320, 400, 512, 640, 720, 768, 800, 960, 1024, 1152,
                        1280, 1440, 1600, 1920};
  int heightOptions[] = {200, 240, 300, 384, 400, 480, 576, 600, 720, 768, 800,
                         864, 900, 960, 1024, 1080, 1200};
  SDL_Rect **resolutions;
  SDL_Rect dst;
  int dimension, count, maxW, maxH;

  dimension = 0;

  if (fullscreen)
    resolutions = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_DOUBLEBUF |
                                      SDL_HWSURFACE | SDL_HWPALETTE   );
  else 
    resolutions = SDL_ListModes(NULL, SDL_RESIZABLE | SDL_DOUBLEBUF |
                                      SDL_HWSURFACE | SDL_HWPALETTE  );

  if (resolutions == (SDL_Rect **)(-1)) {

    maxW = 1920;
    maxH = 1200;

  } else {

    maxW = 320;
    maxH = 200;

    for (count = 0; resolutions[count] != NULL; count++) {

      if (resolutions[count]->w > maxW) maxW = resolutions[count]->w;
      if (resolutions[count]->h > maxH) maxH = resolutions[count]->h;

    }

  }

  while (1) {

    if (loop() == QUIT) return QUIT;

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      return SUCCESS;

    }

    count = 0;

    SDL_FillRect(screen, NULL, 0);


    // Show screen corners
    dst.w = 32;
    dst.h = 32;
    dst.x = 0;
    dst.y = 0;
    SDL_FillRect(screen, &dst, 79);
    dst.x = screenW - 32;
    SDL_FillRect(screen, &dst, 79);
    dst.y = screenH - 32;
    SDL_FillRect(screen, &dst, 79);
    dst.x = 0;
    SDL_FillRect(screen, &dst, 79);


    showString("X", (screenW >> 2) + 40, screenH >> 1, fontmn2);

    if (dimension == 0)
      scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

    showNumber(screenW, (screenW >> 2) + 32, screenH >> 1, fontmn2);

    if (dimension == 0)
      restorePalette(fontmn2->pixels);
    else
      scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

    showNumber(screenH, (screenW >> 2) + 104, screenH >> 1, fontmn2);

    if (dimension != 0)
      restorePalette(fontmn2->pixels);


    if (controls[C_LEFT].state == SDL_PRESSED) {

      releaseControl(C_LEFT);

      dimension = !dimension;

    }

    if (controls[C_RIGHT].state == SDL_PRESSED) {

      releaseControl(C_RIGHT);

      dimension = !dimension;

    }

    if (controls[C_UP].state == SDL_PRESSED) {

      releaseControl(C_UP);

      if ((dimension == 0) && (screenW < maxW)) {

        while (screenW >= widthOptions[count]) count++;

        screenW = widthOptions[count];

      }

      if ((dimension == 1) && (screenH < maxH)) {

        while (screenH >= heightOptions[count]) count++;

        screenH = heightOptions[count];

      }

    }

    if (controls[C_DOWN].state == SDL_PRESSED) {

      releaseControl(C_DOWN);

      if ((dimension == 0) && (screenW > 320)) {

        count = 13;

        while (screenW <= widthOptions[count]) count--;

        screenW = widthOptions[count];
        count = -1;

      }

      if ((dimension == 1) && (screenH > 200)) {

        count = 16;

        while (screenH <= heightOptions[count]) count--;

        screenH = heightOptions[count];
        count = -1;

      }

    }

    // Check for a resolution change
    if (count != 0) {

      if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
      else bgScale = ((screenH - 34) / 100) + 1;

      if (fullscreen) {

        screen = SDL_SetVideoMode(screenW, screenH, 8,
                                  SDL_FULLSCREEN | SDL_DOUBLEBUF |
                                  SDL_HWSURFACE | SDL_HWPALETTE);

      } else {

        screen = SDL_SetVideoMode(screenW, screenH, 8,
                                  SDL_RESIZABLE | SDL_DOUBLEBUF |
                                  SDL_HWSURFACE | SDL_HWPALETTE  );

      }

    }

  }

#endif

  return SUCCESS;

}


int setupMenuLoop (void) {

  char *options[4] = {"CHARACTER",
                      "KEYBOARD",
                      "JOYSTICK",
                      "RESOLUTION"};
  int option, count;

  usePalette(menuPalettes[1]);

  option = 0;

  while (1) {

    if (loop() == QUIT) return QUIT;

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 0);

    for (count = 0; count < 4; count++) {

      if (count == option)
        scalePalette(fontmn2->pixels, F2, (-240 * 2) + 114);

      showString(options[count], screenW >> 2,
                 (screenH >> 1) + (count << 4) - 32, fontmn2);

      if (count == option)
        restorePalette(fontmn2->pixels);

    }

    if (controls[C_UP].state == SDL_PRESSED) {

      releaseControl(C_UP);

      option = (option + 3) % 4;

    }

    if (controls[C_DOWN].state == SDL_PRESSED) {

      releaseControl(C_DOWN);

      option = (option + 1) % 4;

    }

    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      switch (option) {

        case 0:

          setupCharacterMenuLoop();

          break;

        case 1:

          setupKeyboardMenuLoop();

          break;

        case 2:

          setupJoystickMenuLoop();

          break;

        case 3:

          setupResolutionMenuLoop();

          break;

      }

    }

  }

  return SUCCESS;

}


int runMenu (void) {

  int option;
  SDL_Rect src, dst;

  if (loadMenu()) return FAILURE;

  option = 0;

  usePalette(menuPalettes[0]);

  while (1) {

    if (loop() == QUIT) {

      freeMenu();

      return QUIT;

    }

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      freeMenu();

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 28);

    dst.x = (screenW >> 2) - 72;
    dst.y = screenH - (screenH >> 2);
    SDL_BlitSurface(menuScreens[14], NULL, screen, &dst);

    dst.x = (screenW - 320) >> 1;
    dst.y = (screenH - 200) >> 1;
    SDL_BlitSurface(menuScreens[0], NULL, screen, &dst);

    switch (option) {

      case 0:

        src.x = 92;
        src.y = 35;
        src.w = 136;
        src.h = 22;

        break;

      case 1:

        src.x = 92;
        src.y = 57;
        src.w = 140;
        src.h = 22;

        break;

      case 2:

        src.x = 88;
        src.y = 83;
        src.w = 144;
        src.h = 22;

        break;

      case 3:

        src.x = 86;
        src.y = 109;
        src.w = 150;
        src.h = 23;

        break;

      case 4:

        src.x = 82;
        src.y = 137;
        src.w = 156;
        src.h = 26;

        break;

      case 5:

        src.x = 78;
        src.y = 166;
        src.w = 166;
        src.h = 29;

        break;

    }

    dst.x = ((screenW - 320) >> 1) + src.x;
    dst.y = ((screenH - 200) >> 1) + src.y;
    SDL_BlitSurface(menuScreens[1], &src, screen, &dst);

    if (controls[C_UP].state == SDL_PRESSED) {

      releaseControl(C_UP);

      option = (option + 5) % 6;

    }

    if (controls[C_DOWN].state == SDL_PRESSED) {

      releaseControl(C_DOWN);

      option = (option + 1) % 6;

    }


    if (controls[C_ENTER].state == SDL_PRESSED) {

      releaseControl(C_ENTER);

      switch(option) {

        case 0: // New game

          if (newGameMenuLoop() == QUIT) {

            freeMenu();

            return QUIT;

          }

          break;

        case 1: // Load game

          if (loadGameMenuLoop() == QUIT) {

            freeMenu();

            return QUIT;

          }

         break;

        case 2: // Instructions

          if (runScene("instruct.0sc") == QUIT) {

            freeMenu();

            return QUIT;

          }

          break;

        case 3: // Setup options

          if (setupMenuLoop() == QUIT) {

            freeMenu();

            return QUIT;

          }

          break;

        case 4: // Order info

          if (runScene("order.0sc") == QUIT) {

            freeMenu();

            return QUIT;

          }

          break;

        case 5: // Exit

          freeMenu();

          return SUCCESS;

      }

      usePalette(menuPalettes[0]);

    }

  }

  return SUCCESS;

}

