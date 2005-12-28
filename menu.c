
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

  // To do

  //return -1;
  return CONTINUE;

}


void freeMenu (void) {

  // To do

  return;

}


void menuLoop (void) {

  // Temporary menu initialisation

  int option;

  for (option = 0; option < 16; option++) {

    realPalette[option].r = (15 - option) * 8;
    realPalette[option].g = (15 - option) * 17;
    realPalette[option].b = (15 - option) * 8;

  }

  SDL_SetPalette(panelBigFont.pixels, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 16);
  SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 16);

  option = 0;

  while (1) {

    DORETURN(loop(), freeMenu();)

    // To do

    // Temporary menu

    SDL_FillRect(screen, NULL, 31);

    showString("Start", 32, screenH / 6, &panelBigFont);
    showString("Choose world:", 32, (screenH * 2) / 6, &panelBigFont);
    showNumber(nextworld, 208, (screenH * 2) / 6, &panelBigFont);
    showString("Choose level:", 32, (screenH * 3) / 6, &panelBigFont);
    showNumber(nextlevel, 208, (screenH * 3) / 6, &panelBigFont);
    showString("Choose difficulty:", 32, (screenH * 4) / 6, &panelBigFont);
    showNumber(difficulty, 208, (screenH * 4) / 6, &panelBigFont);
    showString("Quit", 32, (screenH * 5) / 6, &panelBigFont);
    showString("*", 16, (screenH * (option + 1)) / 6, &panelBigFont);

    if (keys[K_UP].state == SDL_PRESSED) {

      keys[K_UP].state = SDL_RELEASED;

      option = (option + 4) % 5;

    }

    if (keys[K_DOWN].state == SDL_PRESSED) {

      keys[K_DOWN].state = SDL_RELEASED;

      option = (option + 1) % 5;

    }

    if (keys[K_LEFT].state == SDL_PRESSED) {

      keys[K_LEFT].state = SDL_RELEASED;

      if (option == 1) nextworld = (nextworld + 999) % 1000;
      if (option == 2) nextlevel = (nextlevel + 9) % 10;
      if (option == 3) difficulty = (difficulty + 3) % 4;

    }

    if (keys[K_RIGHT].state == SDL_PRESSED) {

      keys[K_RIGHT].state = SDL_RELEASED;

      if (option == 1) nextworld = (nextworld + 1) % 1000;
      if (option == 2) nextlevel = (nextlevel + 1) % 10;
      if (option == 3) difficulty = (difficulty + 1) % 4;

    }

    if (keys[K_FIRE].state == SDL_PRESSED) {

      keys[K_FIRE].state = SDL_RELEASED;

      if (option == 0) {

        DORETURN(loadNextLevel(), freeMenu();)
        levelLoop();

        for (option = 0; option < 16; option++) {

          realPalette[option].r = (15 - option) * 8;
          realPalette[option].g = (15 - option) * 17;
          realPalette[option].b = (15 - option) * 8;

        }

        SDL_SetPalette(panelBigFont.pixels, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 16);
        SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 16);

        option = 0;

      }

      if (option == 4) {

        return;

      }

    }

  }

  return;

}

