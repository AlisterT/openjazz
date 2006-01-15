
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


#include "OpenJazz.h"


int loadMenu () {

  FILE *f;
  unsigned char *buffer;
  int count, end;

  f = fopenFromPath("menu.000");

  fseek(f, 0, SEEK_END);
  end = ftell(f);
  fseek(f, 0, SEEK_SET);

  // Load the main menu graphics
  loadPalette(menuPalettes[0], f);
  menuScreens[0] = loadSurface(f, menuPalettes[0], 320, 200);
  menuScreens[1] = loadSurface(f, menuPalettes[0], 320, 200);


  // Load the Christmas menu graphics
  if (end > 200000) {

    SDL_FreeSurface(menuScreens[0]);
    SDL_FreeSurface(menuScreens[1]);
    loadPalette(menuPalettes[0], f);
    menuScreens[0] = loadSurface(f, menuPalettes[0], 320, 200);
    menuScreens[1] = loadSurface(f, menuPalettes[0], 320, 200);

  }

  SDL_SetColorKey(menuScreens[0], SDL_SRCCOLORKEY, 0);
  SDL_SetColorKey(menuScreens[1], SDL_SRCCOLORKEY, 0);

  // Load the difficulty graphics
  loadPalette(menuPalettes[1], f);
  menuScreens[2] = loadSurface(f, menuPalettes[1], 320, 200);
  SDL_SetColorKey(menuScreens[2], SDL_SRCCOLORKEY, 0);

  // Load the episode pictures (max. 10 episodes + bonus level)

  loadPalette(menuPalettes[2], f);

  for (count = 0; count < 11; count++) {

    menuScreens[count + 3] = loadSurface(f, menuPalettes[2], 134, 110);

    if (ftell(f) >= end) {

      episodes = ++count;

      for (; count < 11; count++)
        menuScreens[count + 3] = createBlankSurface(menuPalettes[2]);

      fclose(f);

      return CONTINUE;

    }

  }

  // This should never be reached

  episodes = count;

  fclose(f);

  return CONTINUE;

}


void freeMenu (void) {

  int count;

  for (count = 0; count < 14; count++) SDL_FreeSurface(menuScreens[count]);

  return;

}


void newGameDifficultyMenuLoop (void) {

  char *options[4] = {"Easy", "Medium", "Hard", "Turbo"};
  SDL_Rect src, dst;
  int count;

  usePalette(menuPalettes[1]);

  while (1) {

    DORETURN(loop(), )

    SDL_FillRect(screen, NULL, 16);

    for (count = 0; count < 4; count++) {

      if (count == difficulty)
        showString(options[count], screenW >> 2,
                   (screenH >> 1) + (count << 4) - 32, redFontmn2);

      else
        showString(options[count], screenW >> 2,
                   (screenH >> 1) + (count << 4) - 32, fontmn2);

    }

    src.x = (difficulty & 1) * 160;
    src.y = (difficulty & 2) * 50;
    src.w = 160;
    src.h = 100;
    dst.x = (screenW >> 1) - 40;
    dst.y = (screenH >> 1) - 50;
    SDL_BlitSurface(menuScreens[2], &src, screen, &dst);

    if (keys[K_UP].state == SDL_PRESSED) {

      keys[K_UP].state = SDL_RELEASED;
      difficulty = (difficulty + 3) % 4;

    }

    if (keys[K_DOWN].state == SDL_PRESSED) {

      keys[K_DOWN].state = SDL_RELEASED;
      difficulty = (difficulty + 1) % 4;

    }

    if ((keys[K_FIRE].state == SDL_PRESSED) ||
        (keys[K_CHANGE].state == SDL_PRESSED) ) {

      keys[K_FIRE].state = SDL_RELEASED;
      keys[K_CHANGE].state = SDL_RELEASED;

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

      // Load the level
      loadNextLevel();

      // Play the level(s)
      levelLoop();

      // Free the player(s)
      free(players);

      return;

    }

  }

  return;

}


void newGameLevelMenuLoop (void) {

  // Temporary menu initialisation

  int option;

  for (option = 0; option < 16; option++) {

    menuPalettes[1][option].r = (15 - option) * 8;
    menuPalettes[1][option].g = (15 - option) * 17;
    menuPalettes[1][option].b = (15 - option) * 8;

  }

  usePalette(menuPalettes[1]);

  option = 0;

  while (1) {

    DORETURN(loop(), )

    // To do

    // Temporary menu

    SDL_FillRect(screen, NULL, 16);

    showString("Choose world:", 32, screenH / 3, &panelBigFont);
    showNumber(nextworld, 208, screenH / 3, &panelBigFont);
    showString("Choose level:", 32, (screenH << 1) / 3, &panelBigFont);
    showNumber(nextlevel, 208, (screenH << 1) / 3, &panelBigFont);
    showString("*", 16, (screenH * (option + 1)) / 3, &panelBigFont);

    if (keys[K_UP].state == SDL_PRESSED) {

      keys[K_UP].state = SDL_RELEASED;

      option ^= 1;

    }

    if (keys[K_DOWN].state == SDL_PRESSED) {

      keys[K_DOWN].state = SDL_RELEASED;

      option ^= 1;

    }

    if (keys[K_LEFT].state == SDL_PRESSED) {

      keys[K_LEFT].state = SDL_RELEASED;

      if (option) nextlevel = (nextlevel + 9) % 10;
      else nextworld = (nextworld + 999) % 1000;

    }

    if (keys[K_RIGHT].state == SDL_PRESSED) {

      keys[K_RIGHT].state = SDL_RELEASED;

      if (option) nextlevel = (nextlevel + 1) % 10;
      else nextworld = (nextworld + 1) % 1000;

    }

    if ((keys[K_FIRE].state == SDL_PRESSED) ||
        (keys[K_CHANGE].state == SDL_PRESSED) ) {

      keys[K_FIRE].state = SDL_RELEASED;
      keys[K_CHANGE].state = SDL_RELEASED;

      newGameDifficultyMenuLoop();

      // This won't actually load the level, but will set nextlevel and
      // nextworld to their correct values
      if (nextlevel == 99) loadNextLevel();

      usePalette(menuPalettes[1]);

    }

  }

  return;

}


void newGameMenuLoop (void) {

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
  SDL_Rect dst;
  int episode, count;

  for (count = 0; count < 8; count++) {

    // red fontmn2 colours
    menuPalettes[2][count + 120].r = count * 36;
    menuPalettes[2][count + 120].g = 0;
    menuPalettes[2][count + 120].b = 0;

    // fontmn2 colours
    menuPalettes[2][count + 240].r = count * 36;
    menuPalettes[2][count + 240].g = count * 36;
    menuPalettes[2][count + 240].b = count * 36;

  }

  usePalette(menuPalettes[2]);

  episode = 0;

  while (1) {

    DORETURN(loop(), )

    SDL_FillRect(screen, NULL, 16);

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

      if (count == episode)
        showString(options[count], screenW >> 3,
                   (screenH >> 1) + (count << 4) - 92, redFontmn2);

      else
        showString(options[count], screenW >> 3,
                   (screenH >> 1) + (count << 4) - 92, fontmn2);

    }

    if (keys[K_UP].state == SDL_PRESSED) {

      keys[K_UP].state = SDL_RELEASED;

      episode = (episode + 11) % 12;

    }

    if (keys[K_DOWN].state == SDL_PRESSED) {

      keys[K_DOWN].state = SDL_RELEASED;

      episode = (episode + 1) % 12;

    }

    if ((keys[K_FIRE].state == SDL_PRESSED) ||
        (keys[K_CHANGE].state == SDL_PRESSED) ) {

      keys[K_FIRE].state = SDL_RELEASED;
      keys[K_CHANGE].state = SDL_RELEASED;

      if (episode < 6) nextworld = episode * 3;
      else if ((episode >= 6) && (episode < 9)) nextworld = (episode + 1) * 3;
      else if (episode == 9) nextworld = 50;
      else nextworld = 0;
      nextlevel = 0;

      if (episode < 10) newGameDifficultyMenuLoop();

      if (episode == 11) newGameLevelMenuLoop();

      usePalette(menuPalettes[2]);

    }

  }

  return;

}


void loadGameMenuLoop (void) {

  // To do

  return;

}


void setupMenuLoop (void) {

  // To do

  return;

}


void menuLoop (void) {

  int option;
  SDL_Rect src, dst;

  option = 0;

  usePalette(menuPalettes[0]);

  while (1) {

    DORETURN(loop(), freeMenu();)

    SDL_FillRect(screen, NULL, 0);

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


    if (keys[K_UP].state == SDL_PRESSED) {

      keys[K_UP].state = SDL_RELEASED;

      option = (option + 5) % 6;

    }

    if (keys[K_DOWN].state == SDL_PRESSED) {

      keys[K_DOWN].state = SDL_RELEASED;

      option = (option + 1) % 6;

    }


    if ((keys[K_FIRE].state == SDL_PRESSED) ||
        (keys[K_CHANGE].state == SDL_PRESSED) ) {

      keys[K_FIRE].state = SDL_RELEASED;
      keys[K_CHANGE].state = SDL_RELEASED;

      switch(option) {

        case 0: // New game

          newGameMenuLoop();

          break;

        case 1: // Load game

          loadGameMenuLoop();

         break;

        case 2: // Instructions

          DORETURN(loadScene("instruct.0sc"), freeLevel();)
          sceneLoop();
          freeScene();

          break;

        case 3: // Setup options

          setupMenuLoop();

          break;

        case 4: // Order info

          DORETURN(loadScene("order.0sc"), freeLevel();)
          sceneLoop();
          freeScene();

          break;

        case 5: // Exit

          return;

      }

      usePalette(menuPalettes[0]);

    }

  }

  return;

}

