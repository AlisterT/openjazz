
/*
 *
 * font.c
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
 * Deals with the loading, displaying and freeing of screen fonts.
 *
 */


#include "OpenJazz.h"
#include <string.h>


font * loadFont (char * fn) {

  font *ret;
  FILE *f;
  unsigned char *pixels, *character;
  int rle, pos, byte, count, next;
  int chr, width, height, y;

  f = fopenFromPath(fn);

  if (f == NULL) return NULL;

  fseek(f, 19, SEEK_SET);

  ret = malloc(sizeof(font));

  width = fgetc(f) << 2;
  ret->h = fgetc(f) << 1;
  ret->w = malloc(128);
  pixels = malloc(width * ret->h * 128);
  memset(pixels, 0, width * ret->h * 128);

  fseek(f, 23, SEEK_SET);

  ret->w[0] = width >> 1;

  for (chr = 1; chr < 128; chr++) {

    fseek(f, 2, SEEK_CUR);
    next = fgetc(f);

    if (next == -1) break;

    next += fgetc(f) << 8;
    next += ftell(f);

    fseek(f, 1, SEEK_CUR);
    ret->w[chr] = fgetc(f);
    fseek(f, 1, SEEK_CUR);
    height = fgetc(f);

    character = malloc((ret->w[chr] * height) + 1);

    pos = 0;

    while (pos < ret->w[chr] * height) {

      rle = fgetc(f);

      if (rle > 127) {

        byte = fgetc(f);

        for (count = 0; count < (rle & 127); count++) {

          character[pos++] = byte;
          if (pos >= ret->w[chr] * height) break;

        }

      } else if (rle > 0) {

        for (count = 0; count < rle; count++) {

          character[pos++] = fgetc(f);
          if (pos >= ret->w[chr] * height) break;

        }

      } else break;

    }

    character[pos] = character[pos - 1];

    for (y = 0; y < height; y++)
      memcpy(pixels + (((chr * ret->h) + y) * width),
             character + (y * ret->w[chr]) + 1, ret->w[chr]);

    free(character);

    ret->w[chr] += 2;

    fseek(f, next, SEEK_SET);

  }

  for (; chr < 128; chr++) ret->w[chr] = width >> 1;

  ret->pixels = createSurface(pixels, width, ret->h * 128);
  SDL_SetColorKey(ret->pixels, SDL_SRCCOLORKEY, 0);

  if (strcmp(fn, "fontmn1.0fn") == 0) {

   for (count = 0; count < 48; count++) ret->map[count] = 0;
   for (; count < 58; count++) ret->map[count] = count - 26;
   for (; count < 65; count++) ret->map[count] = 0;
   for (; count < 91; count++) ret->map[count] = count - 64;
   for (; count < 97; count++) ret->map[count] = 0;
   for (; count < 123; count++) ret->map[count] = count - 96;
   for (; count < 128; count++) ret->map[count] = 0;

  } else if (strcmp(fn, "fontmn2.0fn") == 0) {

   for (count = 0; count < 43; count++) ret->map[count] = 0;
   ret->map[33] = 89;
   ret->map[34] = 96;
   ret->map[39] = 95;
   ret->map[43] = 87;
   ret->map[44] = 83;
   ret->map[45] = 86;
   ret->map[46] = 84;
   ret->map[47] = 90;
   count = 48;
   for (; count < 58; count++) ret->map[count] = count + 5;
   ret->map[58] = 94;
   ret->map[59] = 93;
   ret->map[60] = 0;
   ret->map[61] = 88;
   ret->map[62] = 0;
   ret->map[63] = 85;
   ret->map[64] = 0;
   count = 65;
   for (; count < 91; count++) ret->map[count] = count - 64;
   for (; count < 97; count++) ret->map[count] = 0;
   for (; count < 123; count++) ret->map[count] = count - 70;
   for (; count < 128; count++) ret->map[count] = 0;

  } else {

   for (count = 0; count < 40; count++) ret->map[count] = 0;
   for (; count < 42; count++) ret->map[count] = count + 25;
   for (; count < 48; count++) ret->map[count] = 0;
   ret->map[44] = 101;
   ret->map[45] = 104;
   ret->map[46] = 102;
   for (; count < 58; count++) ret->map[count] = count + 5;
   for (; count < 65; count++) ret->map[count] = 0;
   ret->map[63] = 103;
   for (; count < 91; count++) ret->map[count] = count - 64;
   for (; count < 97; count++) ret->map[count] = 0;
   for (; count < 123; count++) ret->map[count] = count - 70;
   for (; count < 128; count++) ret->map[count] = 0;

  }

  fclose(f);

  return ret;

}



void freeFont (font * f) {

  SDL_FreeSurface(f->pixels);
  free(f->w);
  free(f);

  return;

}



int showString (char * s, int x, int y, font * f) {

  SDL_Rect src, dst;
  int count, offset;

  // Determine the characters' dimensions
  src.x = 0;
  src.h = f->h;

  // Determine the postion at which to draw the first character
  offset = x;

  // Go through each character of the string
  for (count = 0; count < strlen(s); count++) {

    // Determine the character's position on the screen
    src.w = f->w[(int)(f->map[(int)(s[count])])];
    dst.y = y;
    dst.x = offset;

    // Determine the character's position in the font
    if (s[count] >= 0) src.y = f->map[(int)(s[count])] * f->h;
    else src.y = 0;

    // Draw the character to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);

    offset += f->w[(int)(f->map[(int)(s[count])])];

  }

  return offset;

}



void showNumber (int n, int x, int y, font * f) {

  SDL_Rect src, dst;
  int count, offset;

  // Determine the characters' dimensions
  src.x = 0;
  src.h = f->h;

  // n being 0 is a special case. It must not be considered to be a trailing
  // zero, as these are not displayed.
  if (!n) {

    // Determine 0's position on the screen
    src.w = f->w[(int)(f->map['0'])];
    dst.y = y;
    dst.x = x - src.w;

    // Determine 0's position in the font
    src.y = f->map['0'] * f->h;

    // Draw 0 to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);

    return;

  }

  // Determine the length of the number to be drawn
  if (n > 0) count = n;
  else count = -n;

  // Determine the position at which to draw the lowest digit
  offset = x;

  while (count) {

    // Determine the digit's position on the screen
    src.w = f->w[(int)(f->map['0' + (count % 10)])];
    offset -= src.w;
    dst.y = y;
    dst.x = offset;

    // Determine the digit's position in the font
    src.y = f->map['0' + (count % 10)] * f->h;

    // Draw the digit to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);

    count /= 10;

  }

  // If needed, draw the negative sign
  if (n < 0) {

    // Determine the negative sign's position on the screen
    src.w = f->w[(int)(f->map['-'])];
    dst.y = y;
    dst.x = offset - src.w;

    // Determine the negative sign's position on the screen
    src.y = f->map['-'] * f->h;

    // Draw the negative sign to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);

  }

  return;

}


