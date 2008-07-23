
/*
 *
 * util.c
 * Created on the 22nd of July 2008 from parts of main.c
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
 * Contains core utility functions.
 *
 */


#include "OpenJazz.h"
#include <string.h>


FILE * fopenFromPath (char * fileName) {

  FILE *f;
  char *filePath;

  // Allocate space for the file path
  filePath = malloc(strlen(path) + strlen(fileName) + 1);

  // Create the file path
  strcpy(filePath, path);
  strcat(filePath, fileName);

  printf("Opening %s\n", filePath);

  // Open the file
  f = fopen(filePath, "rb");

  // Free the file path
  free(filePath);

  return f;

}


int fileExists (char * fileName) {

  FILE *f;

  printf("Check: ");

  f = fopenFromPath(fileName);

  if (f == NULL) return 0;

  fclose(f);

  return 1;

}


signed long int loadInt (FILE *f) {

  unsigned long int val;

  val = fgetc(f);
  val += fgetc(f) << 8;
  val += fgetc(f) << 16;
  val += fgetc(f) << 24;

  return *((signed long int *)&val);

}


void storeInt (signed long int val, FILE *f) {

  unsigned long int uval;

  uval = *((unsigned long int *)&val);

  fputc(uval & 255, f);
  fputc((uval >> 8) & 255, f);
  fputc((uval >> 16) & 255, f);
  fputc(uval >> 24, f);

  return;

}


unsigned char * loadRLE (FILE *f, int length) {

  char *buffer;
  int rle, pos, byte, count, next;

  // Determine the byte that follows the block
  next = fgetc(f);
  next += fgetc(f) << 8;
  next += ftell(f);

  // Allocate memory for the decompressed data
  buffer = malloc(length);

  pos = 0;

  while (pos < length) {

    rle = fgetc(f);

    if (rle > 127) {

      byte = fgetc(f);

      for (count = 0; count < (rle & 127); count++) {

        buffer[pos++] = byte;
        if (pos >= length) break;

      }

    } else if (rle > 0) {

      for (count = 0; count < rle; count++) {

        buffer[pos++] = fgetc(f);
        if (pos >= length) break;

      }

    } else break;

  }

  fseek(f, next, SEEK_SET);

  return buffer;

}


void skipRLE (FILE *f) {

  int next;

  next = fgetc(f);
  next += fgetc(f) << 8;

  fseek(f, next, SEEK_CUR);

  return;

}


char * loadString (FILE *f) {

  char *string;
  int length, count;

  length = fgetc(f);

  if (length) {

    string = malloc(length + 1);
    for (count = 0; count < length; count++) string[count] = fgetc(f);
    string[length] = 0;

  } else {

    // If the length is not given, assume it is an 8.3 (or 7.3, etc) file name
    string = malloc(13);
    for (count = 0; (count < 8) && (!length); count++) {

      string[count] = fgetc(f);
      if (string[count] == '.') length = ~0;

    }
    string[count++] = fgetc(f);
    string[count++] = fgetc(f);
    string[count++] = fgetc(f);
    string[count] = 0;

  }

  return string;

}


SDL_Surface * createSurface (unsigned char * pixels, int width, int height) {

  SDL_Surface *ret;
  int y;

  // Create the surface
  ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);

  // Set the surface's palette
  SDL_SetPalette(ret, SDL_LOGPAL, logicalPalette, 0, 256);

  // Upload pixel data to the surface
  if (SDL_MUSTLOCK(ret)) SDL_LockSurface(ret);
  for (y = 0; y < height; y++)
    memcpy(ret->pixels + (ret->pitch * y), pixels + (width * y), width);
  if (SDL_MUSTLOCK(ret)) SDL_UnlockSurface(ret);

  // Free redundant pixel data
  free(pixels);

  return ret;

}


SDL_Surface * createBlankSurface (void) {

  SDL_Surface *ret;
  unsigned char *pixels;

  pixels = malloc(1);
  *pixels = 0;
  ret = createSurface(pixels, 1, 1);
  SDL_SetColorKey(ret, SDL_SRCCOLORKEY, 0);

  return ret;

}


void loadPalette (SDL_Color *palette, FILE *f) {

  unsigned char *buffer;
  int count;

  buffer = loadRLE(f, 768);

  for (count = 0; count < 256; count++) {

    // Palette entries are 6-bit
    // Shift them upwards to 8-bit, and fill in the lower 2 bits
    palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
    palette[count].g = (buffer[(count * 3) + 1] << 2) +
                       (buffer[(count * 3) + 1] >> 4);
    palette[count].b = (buffer[(count * 3) + 2] << 2) +
                       (buffer[(count * 3) + 2] >> 4);

  }

  free(buffer);

  return;

}


void usePalette (SDL_Color *palette) {

  // Make palette changes invisible until the next draw. Hopefully.
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  SDL_Flip(screen);

  SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
  currentPalette = palette;

  return;

}


void scalePalette (SDL_Surface *surface, fixed scale, signed int offset) {

  SDL_Color palette[256];
  int count;

  for (count = 0; count < 256; count++) {

    palette[count].r = ((count * scale) >> 10) + offset;
    palette[count].g = ((count * scale) >> 10) + offset;
    palette[count].b = ((count * scale) >> 10) + offset;

  }

  SDL_SetPalette(surface, SDL_LOGPAL, palette, 0, 256);

  return;

}


void restorePalette (SDL_Surface *surface) {

  SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

  return;

}



