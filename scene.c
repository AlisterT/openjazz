
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


SDL_Surface *sceneBGs[1];
SDL_Color    scenePalette[256];
int          sceneMusic;


int loadScene (char * fn) {


  FILE *f;
  unsigned char *buffer;
  int count, next, type;

  sceneMusic = 0;

  f = fopenFromPath(fn);

  if (f == NULL) return FAILURE;

  // Skip header text
  fseek(f, 19, SEEK_SET);

  next = fgetc(f);
  next += fgetc(f) << 8;

  fseek(f, 25, SEEK_SET);
  fseek(f, fgetc(f), SEEK_SET);

  next += ftell(f) - 15;

  // At this point, next bytes should be 0x50 0x01 0x00 0x00 0x00
  // Then, (0x3f 0x02)
  //    OR (Nothing)
  // Then, (0x2a
  //        Then the length of the music file name
  //        Then the bytes of the music file name)
  //    OR (0x3f, then another byte)
  //    OR (0x4c, not followed by any font stuff)
  //    OR (0xa6, then 20 bytes?)
  //    OR (Nothing)
  // Then 0x58 0x01 0x00
  // Then the length of a font file name
  // Then a font file name
  // Then 0x58 0x02 0x00
  // Then the length of a font file name
  // Then a font file name

  fseek(f, 5, SEEK_CUR);
  type = fgetc(f);

  while (type == 0x3f) {

    fseek(f, 1, SEEK_CUR);
    type = fgetc(f);

  }

  if (type != 0x4C) {

    if (type == 0x2A) {

      // Music file name
      buffer = loadString(f);
      if (loadMusic(buffer) == SUCCESS) sceneMusic = -1;
      free(buffer);

    } else if (type == 0x63) {

      fseek(f, 1, SEEK_CUR);

    } else if (type == 0xA6) {

      fseek(f, 20, SEEK_CUR);

    } else fseek(f, -1, SEEK_CUR); // type should be 58

    while (fgetc(f) == 0x58) {

      // Font names (file names minus extensions)
      fseek(f, 2, SEEK_CUR);
      buffer = loadString(f);

    }

  }

  fseek(f, -1, SEEK_CUR);

  while (fgetc(f) == 0x3f) {

    fseek(f, 1, SEEK_CUR);

  }

  fseek(f, -1, SEEK_CUR);

  // Then 0x4c 0x00 0x00 0x00 0x00 0x01 0x00
  // Then, (0x46
  //        Then a small number, e.g. 0x01, 0x02
  //        Then 0x00 0x4a
  //        Then (0x02 0x5d)
  //          OR (0x01 0xdb)
  //    OR (0x57 0x14...)

//  printf("Initial search reached %d\n", ftell(f));

  // Skip to the palette
  fseek(f, next, SEEK_SET);

  // Load the palette

//  printf("Palette at %d\n", ftell(f));

  loadPalette(scenePalette, f);

  usePalette(scenePalette);

  fseek(f, 4, SEEK_CUR);

//  printf("Pixels at %d\n", ftell(f));

  sceneBGs[0] = loadSurface(f, 320, 200);

  fclose(f);

  return SUCCESS;

}


void freeScene (void) {

  // To do

  SDL_FreeSurface(sceneBGs[0]); // Temporary

  if (sceneMusic) freeMusic();

  return;

}



int runScene (char * fn) {

  SDL_Rect dst;

  if (loadScene(fn)) return FAILURE;

  while (1) {

    if (loop() == QUIT) {

      freeScene();

      return QUIT;

    }

    if ((controls[C_ENTER].state == SDL_PRESSED) ||
        (controls[C_ESCAPE].state == SDL_PRESSED)  ) {

      releaseControl(C_ENTER);
      releaseControl(C_ESCAPE);

      freeScene();

      return SUCCESS;

    }

    // Temporary stuff
    SDL_FillRect(screen, NULL, 31);
    dst.x = (screenW - 320) >> 1;
    dst.y = (screenH - 200) >> 1;
    SDL_BlitSurface(sceneBGs[0], NULL, screen, &dst);
    SDL_Delay(20);

  }

  return SUCCESS;

}
