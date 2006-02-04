
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
#include <time.h>


SDL_Surface *menuScreens[15];
SDL_Color    menuPalettes[4][256];
int          episodes;


int loadMenu () {

  FILE *f;
  time_t currentTime;
  unsigned char *buffer;
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


int newGameDifficultyMenuLoop (void) {

  char *options[4] = {"Easy", "Medium", "Hard", "Turbo"};
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
      localPlayer = players + 0;

      // Set the player's defaults
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
  SDL_Color palette[256];

  for (option = 0; option < 16; option++) {

    palette[option].r = (15 - option) * 12;
    palette[option].g = (15 - option) * 17;
    palette[option].b = (15 - option) * 12;

  }

  memset(palette + 16, 0, sizeof(SDL_Color) * 240);

  usePalette(palette);

  world = level = option = 0;

  while (1) {

    if (loop()) return QUIT;

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      return SUCCESS;

    }

    SDL_FillRect(screen, NULL, 15);

    showString("Choose world:", 32, screenH / 3, &panelBigFont);
    showNumber(world, 208, screenH / 3, &panelBigFont);
    showString("Choose level:", 32, (screenH << 1) / 3, &panelBigFont);
    showNumber(level, 208, (screenH << 1) / 3, &panelBigFont);
    showString(".", 16, (screenH * (option + 1)) / 3, &panelBigFont);

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

      restorePalette(fontmn2->pixels);

      free(nextLevel);
      nextLevel = malloc(11);
      sprintf(nextLevel, "level%1i.%03i", level, world);

      if (newGameDifficultyMenuLoop() == QUIT) return QUIT;

      usePalette(palette);

    }

  }

  return SUCCESS;

}


int newGameMenuLoop (void) {

  char *options[12] = {"Episode one",
                       "Episode two",
                       "Episode three",
                       "Episode four",
                       "Episode five",
                       "Episode six",
                       "Episode A",
                       "Episode B",
                       "Episode C",
                       "Episode X",
                       "Bonus stage",
                       "Specific level"};
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

      return;

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


int setupMenuLoop (void) {

  // To do

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

