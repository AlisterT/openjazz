
/*
 *
 * scene.c
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
 * Deals with the loading, displaying and freeing of the cutscenes.
 *
 */


#include "OpenJazz.h"


int loadScene (char * fn) {


  FILE *f;
  unsigned char *buffer;
  int count;

  f = fopenFromPath(fn);

  fseek(f, 19, SEEK_SET);

  // Skip a mystery block
  skipRLE(f);


  // Load the palette

  fseek(f, 5, SEEK_CUR);
  loadPalette(cutscene.palette, f);

  // Apply the palette to surfaces that already exist, e.g. fonts
  usePalette(cutscene.palette);

  fseek(f, 4, SEEK_CUR);

  cutscene.pixels = loadSurface(f, cutscene.palette, 320, 200);

  fclose(f);

  return CONTINUE;

}


void freeScene (void) {

  // To do

  SDL_FreeSurface(cutscene.pixels); // Temporary

  return;

}



void sceneLoop (void) {

  while (1) {

    DORETURN(loop(), freeScene();)

    // Temporary stuff
    SDL_FillRect(screen, NULL, 31);
    SDL_BlitSurface(cutscene.pixels, NULL, screen, NULL);
    SDL_Delay(50);

    if (keys[K_FIRE].state == SDL_PRESSED) {

        keys[K_FIRE].state = SDL_RELEASED;

        return;

    }

  }

  return;

}
