
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

  // To do

  return NULL;

}



void freeFont (font * f) {

  SDL_FreeSurface(f->pixels);
  free(f);

  return;

}



void showString (char * s, int x, int y, font * f) {

  SDL_Rect src, dst;
  int i;

  // Determine the characters' dimensions
  src.x = 0;
  src.w = f->w;
  src.h = f->h;

  // Go through each character of the string
  for (i = 0; i < strlen(s); i++) {

    // Determine the character's position on the screen
    dst.y = y;
    dst.x = x + (i * f->w);

    // Determine the character's position in the font
    if (s[i] >= 0) src.y = f->map[(int)(s[i])] * f->h;
    else src.y = 0;

    // Draw the character to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);

  }

  return;

}



void showNumber (int n, int x, int y, font * f) {

  SDL_Rect src, dst;
  int i, offset;

  // Determine the characters' dimensions
  src.x = 0;
  src.w = f->w;
  src.h = f->h;

  // n being 0 is a special case. It must not be considered to be a trailing
  // zero, as these are not displayed.
  if (n == 0) {

    // Determine 0's position on the screen
    dst.y = y;
    dst.x = x - f->w;

    // Determine 0's position in the font
    src.y = f->map['0'] * f->h;

    // Draw 0 to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);

    return;

  }

  // Determine the length of the number to be drawn
  if (n > 0) i = n;
  else i = -n;

  // Determine the position at which to draw the lowest digit
  offset = x - f->w;

  while (i != 0) {

    // Determine the digit's position on the screen
    dst.y = y;
    dst.x = offset;

    // Determine the digit's position in the font
    src.y = f->map['0' + i - ((i / 10) * 10)] * f->h;

    // Draw the digit to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);
    i = i / 10;

    // Determine the position at which to draw the next lowest digit
    offset -= f->w;

  }

  // If needed, draw the negative sign
  if (n < 0) {

    // Determine the negative sign's position on the screen
    dst.y = y;
    dst.x = offset;

    // Determine the negative sign's position on the screen
    src.y = f->map['-'] * f->h;

    // Draw the negative sign to the screen
    SDL_BlitSurface(f->pixels, &src, screen, &dst);

  }

  return;

}


