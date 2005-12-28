
/*
 *
 * level.c
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
 * Deals with the loading, running and freeing of ordinary levels.
 *
 */


#include "OpenJazz.h"
#include <string.h>


char *sceneFile;
int   nPlayers = 1;
int   time;
int   skyOrb;


int loadSprites (char * fn) {

  FILE *f, *mf, *sf;
  unsigned char *pixels, *sorted;
  int mposition, sposition;
  int i, x, y, w, h, m; // VERY VERY BAD VARIABLE NAMES
                        // But cleaning up this little coding faux-pas would
                        // take ages.

  // Open fn
  f = sf = fopenFromPath(fn);

  // This function loads all the sprites, not fust those in fn
  // Note: Lower case is necessary for Unix support
  mf = fopenFromPath("mainchar.000");

  // Find where the sprites start in fn
  sposition = 2;
  sposition += fgetc(f) << 1;
  sposition += fgetc(f) << 1;

  // Find where the sprites start in mainchar.000
  mposition = 2;

  // Loop through all the sprites to be loaded
  for (i = 0; i < SPRITES; i++) {

    // Go to the start of the current sprite or file indicator
    fseek(sf, sposition, SEEK_SET);
    fseek(mf, mposition, SEEK_SET);

    // If both fn and mainchar.000 have file indicators, create a blank sprite
    while ((fgetc(sf) == 0xFF) && (fgetc(mf) == 0xFF)) {

      // Go to the next sprite/file indicator
      fseek(sf, 1, SEEK_CUR);
      fseek(mf, 1, SEEK_CUR);

      // set the position of the next sprite/file indicators
      sposition += 2;
      mposition += 2;

      // Create a blank sprite
      pixels = malloc(1);
      *pixels = SKEY;
      spriteSet[i] = createSurface(pixels, 1, 1);
      SDL_SetColorKey(spriteSet[i], SDL_SRCCOLORKEY, SKEY);
      i++;

    }

    // Return to the start of the sprite/file indicators
    fseek(sf, sposition, SEEK_SET);
    fseek(mf, mposition, SEEK_SET);

    // Unless otherwise stated, load from fn
    f = sf;

    // Check if otherwise stated
    if (fgetc(f) == 0xFF) {

      f = mf;

    } else fseek(f, -1, SEEK_CUR);

    // Width
    w = fgetc(f) << 2;
    fseek(f, 1, SEEK_CUR);

    // Height
    h = fgetc(f);
    fseek(f, 1, SEEK_CUR);

    // Position of the next sprite or file indicator in each file
    if (f == sf) {

      mposition += 2;

      sposition += 10 + (fgetc(f) << 2);
      sposition += fgetc(f) << 10;

    } else {

      sposition += 2;

      mposition += 10 + (fgetc(f) << 2);
      mposition += fgetc(f) << 10;

    }

    // m is for MAGIC
    m = fgetc(f);

    // Allocate space for pixel data
    pixels = malloc(w * h);

    // Allocate space for descrambling
    sorted = malloc(w * h);

    // Actually, m is for mask offset.
    // Sprites can be either masked or not masked.
    if (m == 0) {

      // Not masked
      // Load the pixel data directly for descrambling

      fseek(f, 3, SEEK_CUR);

      fread(pixels, w, h, f);


    } else {

      // Masked
      // Load the pixel data according to the mask

      fseek(f, 1, SEEK_CUR);

      // Masked sprites have their own next sprite offsets
      if (f == sf) {

        sposition = fgetc(f) << 2;
        sposition += fgetc(f) << 10;

      } else {

        mposition = fgetc(f) << 2;
        mposition += fgetc(f) << 10;

      }

      // Skip to mask
      fseek(f, m, SEEK_CUR);

      // Read the mask
      // Each mask pixel is either 0 or 1
      // Four pixels are packed into the lower end of each byte
      for (y = 0; y < h; y++) {

        for (x = 0; x < w; x++) {

          if ((x & 3) == 0) m = fgetc(f);
          pixels[(y * w) + x] = (m >> (x & 3)) & 1;

        }

      }

      // Pixels are loaded if the corresponding mask pixel is 1, otherwise the
      // transparent index is used. Pixels are scrambled, so the mask has to be
      // scrambled the same way.
      for (y = 0; y < h; y++) {

        for (x = 0; x < w; x++) {

          sorted[(((y >> 2) + ((x & 3) * (h >> 2))) * w) + (x >> 2) + (((y & 3) + ((h & 3) * (x & 3))) * (w >> 2))] = pixels[(y * w) + x];

        }

      }

      // Skip to pixels
      fseek(f, (w / 4), SEEK_CUR);

      // Next sprite offsets are relative to here
      if (f == sf) sposition += ftell(f);
      else mposition += ftell(f);

      // Read pixels according to the scrambled mask
      for (y = 0; y < h; y++) {

        for (x = 0; x < w; x++) {

          if (sorted[(y * w) + x] == 1) {

            // The unmasked portions are transparent, so no masked portion
            // should be transparent.
            m = SKEY;
            while (m == SKEY) m = fgetc(f);

            // Use the acceptable pixel
            pixels[(y * w) + x] = m;

          } else pixels[(y * w) + x] = SKEY; // Use the transparent pixel

        }

      }


    }

    // Rearrange pixels in correct order
    for (y = 0; y < h; y++) {

      for (x = 0; x < w; x++) {

        sorted[(y * w) + x] = pixels[(((y >> 2) + ((x & 3) * (h >> 2))) * w) + (x >> 2) + (((y & 3) + ((h & 3) * (x & 3))) * (w >> 2))];

      }

    }


    // Convert the sprite to an SDL surface
    spriteSet[i] = createSurface(sorted, w, h);
    SDL_SetColorKey(spriteSet[i], SDL_SRCCOLORKEY, SKEY);

    // Free redundant data
    free(pixels);

    // Check if the next sprite exists
    // If not, create blank sprites for the remainder
    fseek(sf, sposition, SEEK_SET);

    if (fgetc(sf) == -1) {

      for (i++; i < SPRITES; i++) {

        pixels = malloc(1);
        *pixels = SKEY;
        spriteSet[i] = createSurface(pixels, 1, 1);
        SDL_SetColorKey(spriteSet[i], SDL_SRCCOLORKEY, SKEY);

      }

    }

  }

  fclose(mf);
  fclose(sf);

  return CONTINUE;

}


int loadTiles (char * fn) {

  FILE *f;
  unsigned char *buffer;
  int rle, pos, index, count;
  int nTiles;


  f = fopenFromPath(fn);


  // Load the palette

  buffer = loadRLE(f, 768);

  for (count = 0; count < 256; count++) {

    // Palette entries are 6-bit
    // Shift them upwards to 8-bit, and fill in the lower 2 bits
    realPalette[count].r = (buffer[count * 3] << 2) +
                           (buffer[count * 3] >> 6);
    realPalette[count].g = (buffer[(count * 3) + 1] << 2) +
                           (buffer[(count * 3) + 1] >> 6);
    realPalette[count].b = (buffer[(count * 3) + 2] << 2) +
                           (buffer[(count * 3) + 2] >> 6);

  }

  free(buffer);

  // Apply the palette to surfaces that already exist, e.g. fonts
  updatePalettes();


  // Load the background palette

  buffer = loadRLE(f, 765);

  for (count = 0; count < 255; count++) {

    // Palette entries are 6-bit
    // Shift them upwards to 8-bit, and fill in the lower 2 bits
    bgPalette[count].r = (buffer[count * 3] << 2) +
                         (buffer[count * 3] >> 6);
    bgPalette[count].g = (buffer[(count * 3) + 1] << 2) +
                         (buffer[(count * 3) + 1] >> 6);
    bgPalette[count].b = (buffer[(count * 3) + 2] << 2) +
                         (buffer[(count * 3) + 2] >> 6);

  }

  free(buffer);


  // Skip the second, identical, background palette

  skipRLE(f);


  // Load the tile pixel indices

  nTiles = 240; // Never more than 240 tiles

  buffer = malloc(nTiles * TW * TH);

  fseek(f, 4, SEEK_CUR);

  pos = 0;

  // Read the RLE pixels
  // loadRLE() cannot be used, for reasons that will become clear
  while (pos < TW * TH * nTiles) {

    rle = fgetc(f);

    if (rle >= 128) {

      index = fgetc(f);

      for (count = 0; count < (rle & 127); count++) buffer[pos++] = index;

    } else if (rle >= 1) {

      for (count = 0; count < rle; count++) buffer[pos++] = fgetc(f);

    } else if (rle == 0) { // This happens at the end of each tile

        // 0 pixels means 1 pixel, apparently
        buffer[pos++] = fgetc(f);

        fseek(f, 2, SEEK_CUR); // I assume this is the length of the next tile
                               // block
        if (pos == TW * TH * 60) fseek(f, 2, SEEK_CUR); // Maybe these say
        if (pos == TW * TH * 120) fseek(f, 2, SEEK_CUR); // whether or not there
        if (pos == TW * TH * 180) fseek(f, 2, SEEK_CUR); // are any more tiles

    } else break; // Probably the end of the file

  }

  fclose(f);

  // Work out how many tiles were actually loaded
  // Should be a multiple of 60
  nTiles = pos / (TW * TH);

  tileSet = createSurface(buffer, TW, TH * nTiles);
  SDL_SetColorKey(tileSet, SDL_SRCCOLORKEY, TKEY);

  return nTiles;

}



int loadLevel (char * fn) {

  FILE *f;
  unsigned char *buffer;
  paletteEffect *pe;
  unsigned char nTiles;
  int count, x, y;


  f = fopenFromPath(fn);

  if (strncasecmp(fn + 6, ".j1l", 4) == 0) {

    // Load level data from a *.j1l file

    // j1l files will be the same as normal level files, but will have full
    // file names for tilesets, etc.

  } else {

    // Load level data from an original Level#.### file

    char *subFN;

    subFN = malloc(12);

    // Load tile set from corresponding blocks.###
    // Note: Lower case is required for Unix support

    // Boss levels are special cases
    if (world == 18) nTiles = loadTiles("blocks.002");
    else if (world == 19) nTiles = loadTiles("blocks.005");
    else if (world == 20) nTiles = loadTiles("blocks.008");
    else if (world == 21) nTiles = loadTiles("blocks.011");
    else if (world == 22) nTiles = loadTiles("blocks.014");
    else if (world == 39) nTiles = loadTiles("blocks.032");
    else if (world == 40) nTiles = loadTiles("blocks.035");
    else if (world == 41) nTiles = loadTiles("blocks.038");
    else {

      // Construct and use file name for non-boss levels
      sprintf(subFN, "blocks.%3s", fn + 7);
      nTiles = loadTiles(subFN);

    }

    // Feline says this byte gives the correct tileset extension. All I get is
    // the level extension.
    fseek(f, -15, SEEK_END);
    x = fgetc(f) ^ 4;


    // Load sprite set from corresponding Sprites.###
    // Note: Lower case is required for Unix support
    sprintf(subFN, "sprites.%3s", fn + 7);
    loadSprites(subFN), free(subFN);

    free(subFN);

    // Skip to tile and event reference data
    fseek(f, 39, SEEK_SET);

  }

  // Load tile and event references

  buffer = loadRLE(f, LW * LH * 2);

  // Create grid from data
  for (x = 0; x < LW; x++) {

    for (y = 0; y < LH; y++) {

      grid[y][x].tile = buffer[(y + (x * LH)) << 1];
      grid[y][x].bg = buffer[((y + (x * LH)) << 1) + 1] >> 7;
      grid[y][x].event = buffer[((y + (x * LH)) << 1) + 1] & 127;

    }

  }

  free(buffer);

  // A mysterious block of mystery
  skipRLE(f);


  // Load mask data

  buffer = loadRLE(f, nTiles * 8);

  // Unpack bits
  for (count = 0; count < nTiles; count++) {

    for (y = 0; y < 8; y++) {

      for (x = 0; x < 8; x++)
        mask[count][(y << 3) + x] = (buffer[(count << 3) + y] >> x) & 1;

    }

  }

  free(buffer);

/* Uncomment the code below if you want to see the mask instead of the tile
   graphics during gameplay */

/*  if (SDL_MUSTLOCK(tileSet)) SDL_LockSurface(tileSet);

  for (pos = 0; pos < nTiles; pos++) {

    for (y = 0; y < TH; y++) {

      for (x = 0; x < TW; x++) {

        if (mask[pos][((y / 4) * 8) + (x / 4)] == 1)
          ((char *)(tileSet->pixels))[(pos * 32 * 32) + (y * 32) + x] = 88;

      }

    }

  }

  if (SDL_MUSTLOCK(tileSet)) SDL_UnlockSurface(tileSet);*/


  // More magical mystery data
  skipRLE(f);


  // Load event set

  buffer = loadRLE(f, EVENTS * ELENGTH);

  for (count = 0; count < EVENTS; count++) {

    for (x = 0; x < ELENGTH; x++)
      eventSet[count][x] = buffer[(count * ELENGTH) + x];

  }

  free(buffer);


  // Eliminate event references for events of too high a difficulty
  for (x = 0; x < LW; x++) {

    for (y = 0; y < LH; y++) {

      if (eventSet[grid[y][x].event][E_DIFFICULTY] > difficulty)
        grid[y][x].event = 0;

    }

  }


  // Yet more doubtless essential data
  skipRLE(f);


  // Load animation set

  buffer = loadRLE(f, ANIMS * 64);

  // Create animation set based on that data
  for (count = 0; count < ANIMS; count++) {

    animSet[count].frames = buffer[(count * 64) + 6];

    for (y = 0; y < animSet[count].frames; y++) {

      // Get frame
      x = buffer[(count * 64) + 7 + y];

      animSet[count].sprites[y] = spriteSet[x];

      // Get vertical offset
      animSet[count].y[y] = buffer[(count * 64) + 45 + y];

    }

  }

  free(buffer);


  // At general data

  // There's a a whole load of unknown data around here

  // Like another one of those pesky RLE blocks
  skipRLE(f);

  // And 505 bytes of DOOM
  fseek(f, 505, SEEK_CUR);

  // Music file
  x = fgetc(f);
  buffer = malloc(x + 1);
  for (y = 0; y < x; y++) buffer[y] = fgetc(f);
  buffer[x] = 0;
  loadMusic(buffer); // It's not as if this even does anything yet
  free(buffer);

  // 25 bytes of undiscovered usefulness, less the music file name
  fseek(f, 25 - x, SEEK_CUR);

  x = fgetc(f);
  if (x == 0) x = 9;
  sceneFile = malloc(x + 1);
  for (y = 0; y < x; y++) sceneFile[y] = fgetc(f);
  sceneFile[x] = 0;

  // 51 bytes of undiscovered usefulness, less the cutscene file name
  fseek(f, 51 - x, SEEK_CUR);

  // Finally, some data I know how to use!

  // First up, the player's coordinates
  // These are grid coordiantes, so they must be shifted into real coordinates
  players[0].x = fgetc(f) << 5;
  players[0].x += fgetc(f) << 13;
  players[0].y = fgetc(f) << 5;
  players[0].y += fgetc(f) << 13;

  // Next level
  nextlevel = fgetc(f);
  nextworld = fgetc(f);

  // Thanks to Feline and the JCS94 team for the next bits:

  // 9 more bytes left in "Section 14"
  fseek(f, 9, SEEK_CUR);

  // Now at "Section 15"


  // Load player's animation set references

  buffer = loadRLE(f, PANIMS * 2);

  for (count = 0; count < PANIMS; count++)
    players[0].anims[count] = buffer[count << 1];

  free(buffer);


  // Now at "Section 16"

  // Skip to "Section 17" - .atk
  fseek(f, 4, SEEK_CUR);

  // And skip it
  skipRLE(f);

  // Now at "Section 18." More skippability.
  skipRLE(f);


  // Now at "Section 19," THE MAGIC SECTION

  // First byte should be the bg mode
  switch (fgetc(f)) {

    case 2:

      // Sky background effect
      bgPE = malloc(sizeof(paletteEffect));
      bgPE->next = NULL;
      bgPE->first = 156;
      bgPE->amount = 100;
      bgPE->type = PE_SKY;
      bgPE->speed = 0.5;
      bgPE->position = 0.0;

      break;

    case 8:

      // Parallaxing background effect
      bgPE = malloc(sizeof(paletteEffect));
      bgPE->next = NULL;
      bgPE->first = 128;
      bgPE->amount = 64;
      bgPE->type = PE_2D;
      bgPE->speed = 0.125;
      bgPE->position = 0.0;

      break;

    case 9:

      // Diagonal stripes "parallaxing" background effect
      bgPE = malloc(sizeof(paletteEffect));
      bgPE->next = NULL;
      bgPE->first = 128;
      bgPE->amount = 32;
      bgPE->type = PE_1D;
      bgPE->speed = 0.5;
      bgPE->position = 0.0;

      break;

    default:

      // No effect, but bgPE must exist so here is a dummy animation
      bgPE = malloc(sizeof(paletteEffect));
      bgPE->next = NULL;
      bgPE->first = 255;
      bgPE->amount = 1;
      bgPE->type = PE_ROTATE;
      bgPE->speed = 1.0;
      bgPE->position = 0.0;

      break;

  }

  fseek(f, 1, SEEK_CUR);

  skyOrb = fgetc(f); // A.k.a the sun, the moon, the brightest star, that red
                     // planet with blue veins...


  // And that's us done!

  fclose(f);


  // Palette animations
  // These are applied to every level without a conflicting background effect
  // As a result, there are a few levels with things animated that shouldn't be

  // In Diamondus: The red/yellow palette animation
  pe = malloc(sizeof(paletteEffect));
  pe->next = bgPE;
  pe->first = 112;
  pe->amount = 4;
  pe->type = PE_ROTATE;
  pe->speed = 32.0;
  pe->position = 0.0;
  firstPE = pe;

  // In Diamondus: The waterfall palette animation
  pe = malloc(sizeof(paletteEffect));
  pe->next = firstPE;
  pe->first = 116;
  pe->amount = 8;
  pe->type = PE_ROTATE;
  pe->speed = 16.0;
  pe->position = 0.0;
  firstPE = pe;

  // The following were discoverd by Unknown/Violet

  pe = malloc(sizeof(paletteEffect));
  pe->next = firstPE;
  pe->first = 124;
  pe->amount = 3;
  pe->type = PE_ROTATE;
  pe->speed = 16.0;
  pe->position = 0.0;
  firstPE = pe;

  if ((bgPE->type != PE_1D) && (bgPE->type != PE_2D)) {

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 132;
    pe->amount = 8;
    pe->type = PE_ROTATE;
    pe->speed = 16.0;
    pe->position = 0.0;
    firstPE = pe;

  }

  if ((bgPE->type != PE_SKY) && (bgPE->type != PE_2D)) {

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 160;
    pe->amount = 32;
    pe->type = PE_ROTATE;
    pe->speed = -16.0;
    pe->position = 0.0;
    firstPE = pe;

  }

  if (bgPE->type != PE_SKY) {

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 192;
    pe->amount = 32;
    pe->type = PE_ROTATE;
    pe->speed = -32.0;
    pe->position = 0.0;
    firstPE = pe;

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 224;
    pe->amount = 16;
    pe->type = PE_ROTATE;
    pe->speed = 16.0;
    pe->position = 0.0;
    firstPE = pe;

  }


  // some intital values for the player

  players[0].jumpStart = 0;
  players[0].walkStart = 0;
  players[0].facing = 1;
  players[0].energy = 4;
  players[0].energyBar = 0;


  // Set the time at which the level will end
  time = SDL_GetTicks() + ((5 - difficulty) * 2 * 60 * 1000);

  return CONTINUE;

}


int loadNextLevel () {

  FILE *f;
  char *fn;
  int count;

  fn = malloc(11);

  if (nextlevel == 99) {

    // End of episode

    // Tell our makeshift menu which level to go for next
    if (world == 18) nextworld = 3;
    if (world == 19) nextworld = 6;
    if (world == 20) nextworld = 9;
    if (world == 21) nextworld = 12;
    if (world == 22) nextworld = 15;
    if (world == 23) nextworld = 30;
    if (world == 39) nextworld = 33;
    if (world == 40) nextworld = 36;
    if (world == 41) nextworld = 50;
    if (world == 51) nextworld = 0; // Holiday Hare
    if (world == 52) nextworld = 0; // Christmas Edition
    nextlevel = 0;

    // Tell levelLoop to return to the menu
    return RETURN;

  } else {

    world = nextworld;
    level = nextlevel;

  }

  // Load whichever level nextlevel and nextworld refer to
  sprintf(fn, "level%1d.%03d", level, world);
  f = fopenFromPath(fn);

  // If that didn't work, scan for level
  for (count = 0; (count < 156) && (f == NULL); count++) {

    if (level >= 2) {

      world++;
      if (world == 53) world = 0;
      level = 0;

    } else level++;

    if (count == 30) {

      world = 50;
      level = 0;

    }

    sprintf(fn, "level%1d.%03d", level, world);
    f = fopenFromPath(fn);

  }

  if (f != NULL) {

    // if the scan was successful

    fclose(f);
    count = loadLevel(fn);
    free(fn);

  } else {

    // if the scan was unsuccessful

    free(fn);
    fprintf(stderr, "Could not find any levels in %s\n", path);

    return RETURN;

  }

  // Return the return value from loadLevel()
  return count;

}


int loadMacro (char * fn) {

  // To do

  return -1;

}


void freeLevel (void) {

  paletteEffect *pe;
  int count;

  // Free the palette effects
  while(firstPE != NULL) {

    pe = firstPE->next;
    free(firstPE);
    firstPE = pe;

  }

  // Free the tile set
  SDL_FreeSurface(tileSet);

  // Free the sprites, remembering to include the last blank one
  for (count = 0; count < SPRITES; count++) SDL_FreeSurface(spriteSet[count]);

  // Free the cutscene file name
  free(sceneFile);

  return;

}



int checkMask (int x, int y) {

  // Anything off the edge of the map is solid
  if ((x < 0) || (y < 0) || (x > LW * TW) || (y > LH * TH)) return 1;

  // If the player is going upwards through one-way scenery, pretend it is not
  // solid
  if ((players[0].rising != 0) && (grid[y >> 5][x >> 5].event) == 122) return 0;

  // Check the mask in the tile in question
  return mask[grid[y >> 5][x >> 5].tile]
             [(((y & 31) >> 2) << 3) + ((x & 31) >> 2)];

}



void processEvent (int x, int y, int frame) {

  SDL_Rect dst;
  gridElement *ge;
  unsigned char *event;

  // To do: A whole lot

  // Get the grid element at the given coordinates
  ge = grid[y + ((int)players[0].viewY >> 5)] + x +
       ((int)players[0].viewX >> 5);

  // Get the event used by the grid element
  event = eventSet[ge->event];

  switch (ge->event) {

    case 0: // No event
    case 122: // One-way
    case 123: // Foreground
    case 124: // Foreground
    case 125: // Foreground
    case 126: // ...Something

      break;

    default:

      // What follows is BRIMMING WITH EVIL

      // Find the horizontal position at which to draw the event
      dst.x = (x << 5) - ((int)players[0].viewX & 31);

      // Decide whether to use the left or the right animation, based on whether
      // the player is to the left or to the right of the event
      if (players[0].x < ((x + ((int)players[0].viewX >> 5)) << 5)) {

        // Ensure that the animation reference is valid
        if ((event[E_LEFTANIM] > 0) && (event[E_LEFTANIM] < ANIMS)  ) {

          // Find the vertical position at which to draw the event
          dst.y = animSet[event[E_LEFTANIM]].
                  y[frame % animSet[event[E_LEFTANIM]].frames]
                  - animSet[event[E_LEFTANIM]].
                  sprites[frame % animSet[event[E_LEFTANIM]].frames]->h
                  + ((y + 1) << 5) - ((int)players[0].viewY & 31);

          // Draw the event
          SDL_BlitSurface(animSet[event[E_LEFTANIM]].
                          sprites[frame % animSet[event[E_LEFTANIM]].frames],
                          NULL, screen, &dst);

        }

      } else {

        // Ensure that the animation reference is valid
        if ((event[E_RIGHTANIM] > 0) && (event[E_RIGHTANIM] < ANIMS)  ) {

          // Find the vertical position at which to draw the event
          dst.y = animSet[event[E_RIGHTANIM]].
                  y[frame % animSet[event[E_RIGHTANIM]].frames]
                  - animSet[event[E_RIGHTANIM]].
                  sprites[frame % animSet[event[E_RIGHTANIM]].frames]->h
                  + ((y + 1) << 5) - ((int)players[0].viewY & 31);

          // Draw the event
          SDL_BlitSurface(animSet[event[E_RIGHTANIM]].
                          sprites[frame % animSet[event[E_RIGHTANIM]].frames],
                          NULL, screen, &dst);

        }

      }

      break;

  }

  return;

}



void levelLoop (void) {

  gridElement *ge;
  SDL_Rect src, dst;
  float dx, dy;
  int x, y, ticks, frame;
  float smoothfps;

  // To do: Authentic physics, macro handling, other stuff

  // Arbitrary initial value
  smoothfps = 50.0f;

  while (1) {

    // Do general processing
    DORETURN(loop(), freeLevel();)

    if (fps) {

      smoothfps = smoothfps + 1 - (smoothfps / fps);
      /* This eqaution is a simplified version of
         (fps * c) + (smoothfps * (1 - c))
         where c = (1 / fps)
         In other words, the response of smoothFPS to changes in FPS
         decreases as the framerate increases 
         The following version is for c = (1 / smoothfps)
         //      smoothfps = (fps / smoothfps) + smoothfps - 1;
         I bet about a week after I came up with this, I'll find out that it's
         been around for years. */

      // Ignore outlandish values
      if (smoothfps > 999) smoothfps = 999;
      if (smoothfps < 1) smoothfps = 1;

    }

    // Cheaper than calling SDL_GetTicks() whenever the time is needed
    ticks = SDL_GetTicks();

    // The current frame for animations
    frame = ticks / 75;


    // Physics

    // Determine the player's motion

    if (keys[K_LEFT].state == SDL_PRESSED) {
    
      if ((players[0].walkStart == 0) || (players[0].facing == 1))
        players[0].walkStart = ticks;

      players[0].facing = 0;

    }

    if ((keys[K_LEFT].state == SDL_RELEASED) && (players[0].facing == 0)) {

       players[0].walkStart = 0;

    }
    
    if (keys[K_RIGHT].state == SDL_PRESSED) {
    
      if ((players[0].walkStart == 0) || (players[0].facing == 0))
        players[0].walkStart = ticks;

      players[0].facing = 1;

    }

    if ((keys[K_RIGHT].state == SDL_RELEASED) && (players[0].facing == 1)) {

       players[0].walkStart = 0;

    }
    
    if ((keys[K_JUMP].state == SDL_PRESSED) &&
        (checkMask(players[0].x + 12, players[0].y + 33) ||
         checkMask(players[0].x + 16, players[0].y + 33) ||
         checkMask(players[0].x + 20, players[0].y + 33)   ) &&
        !checkMask(players[0].x + 16, players[0].y - 1) &&
        (players[0].jumpStart == 0) && (players[0].rising == 0)) {

      players[0].jumpStart = ticks;
      if ((players[0].walkStart) &&
          (players[0].jumpStart - players[0].walkStart > WALKTIME))
        players[0].rising = 2;
      else players[0].rising = 1;

    }

    if ((keys[K_JUMP].state == SDL_RELEASED)
        || checkMask(players[0].x + 16, players[0].y + 11)
        || (ticks - players[0].jumpStart > JUMPTIME)) {

       if (players[0].jumpStart) {

         players[0].walkStart = 0;
         players[0].jumpStart = 0;

       }

       if (players[0].rising <= 2) players[0].rising = 0;

    }
    

    // Calculate the player's trajectory

    if (players[0].rising > 0) dy = spf * PS_JUMP;
    else dy = spf * PS_FALL;

    if (players[0].walkStart > 0) {

      if (players[0].facing == 0) {

        if (ticks - players[0].walkStart > WALKTIME) dx = spf * -PS_RUN;
        else dx = spf * -PS_WALK;

      } else {

        if (ticks - players[0].walkStart > WALKTIME) dx = spf * PS_RUN;
        else dx = spf * PS_WALK;

      }

    } else dx = 0;


    // Apply as much of that trajectory as possible, without going into the
    // scenery
    
    x = (int)dx;
    y = (int)dy;

    // First for the vertical component of the trajectory

    while (y > 0) {

      if (checkMask(players[0].x + 12, players[0].y + 33) ||
          checkMask(players[0].x + 16, players[0].y + 33) ||
          checkMask(players[0].x + 20, players[0].y + 33)   ) break;

      players[0].y++;
      y--;

    }

    while (y < 0) {

      if (checkMask(players[0].x + 16, players[0].y + 11)) break;

      players[0].y--;
      y++;

    }

    dy -= (int)dy;

    if (((dy > 0) &&
        !(checkMask(players[0].x + 12, players[0].y + 32 + dy) ||
          checkMask(players[0].x + 16, players[0].y + 32 + dy) ||
          checkMask(players[0].x + 20, players[0].y + 32 + dy)   )) ||
        ((dy < 0) && !checkMask(players[0].x + 16, players[0].y + 12 + dy)))
      players[0].y += dy;

    // Then for the horizontal component of the trajectory

    while (x < 0) {

      if (checkMask(players[0].x + 5, players[0].y + 13)) break;

      players[0].x--;
      x++;

    }

    while (x > 0) {

      if (checkMask(players[0].x + 27, players[0].y + 13)) break;

      players[0].x++;
      x--;

    }

    dx -= (int)dx;

    if (((dx < 0) && !checkMask(players[0].x + 6 + dx, players[0].y + 13)) ||
        ((dx > 0) && !checkMask(players[0].x + 26 + dx, players[0].y + 13))   )
      players[0].x += dx;

    // Push the player upwards if on an uphill slope
    while (checkMask(players[0].x + 16, players[0].y + 32)) players[0].y--;


    // Choose animation

    if (players[0].rising == 0) {

      if (checkMask(players[0].x + 12, players[0].y + 36) ||
          checkMask(players[0].x + 16, players[0].y + 36) ||
          checkMask(players[0].x + 20, players[0].y + 36) ||
          checkMask(players[0].x + 12, players[0].y + 44) ||
          checkMask(players[0].x + 16, players[0].y + 44) ||
          checkMask(players[0].x + 20, players[0].y + 44)   ) {

        if (players[0].walkStart) {

          if (ticks - players[0].walkStart > WALKTIME)
            players[0].anim = players[0].anims[PA_LRUN + players[0].facing];
          else
            players[0].anim = players[0].anims[PA_LWALK + players[0].facing];

        } else {

          if (!checkMask(players[0].x + 12, players[0].y + 44) &&
              !checkMask(players[0].x + 8, players[0].y + 40))
            players[0].anim = players[0].anims[PA_LEDGE];
          else if (!checkMask(players[0].x + 20, players[0].y + 44) &&
                   !checkMask(players[0].x + 24, players[0].y + 40))
            players[0].anim = players[0].anims[PA_REDGE];
          else
            players[0].anim = players[0].anims[PA_LSTAND + players[0].facing];

        }

      } else players[0].anim = players[0].anims[PA_LFALL + players[0].facing];

    } else if (players[0].rising == 1)
      players[0].anim = players[0].anims[PA_LJUMP + players[0].facing];

    else if (players[0].rising == 2)
      players[0].anim = players[0].anims[PA_LSPIN + players[0].facing];

    else if (players[0].rising > 2)
      players[0].anim = players[0].anims[PA_LSPRING + players[0].facing];


    // Calculate viewport dimensions

    // Can we can see below the panel?
    if (players[0].viewW > panel->w) players[0].viewH = screenH;
    else players[0].viewH = screenH - 33;
    players[0].viewW = screenW;

    players[0].viewX = players[0].x + 8 - (players[0].viewW / 2);
    players[0].viewY = players[0].y + 8 - (players[0].viewH / 2);

    if (players[0].viewX < 0) players[0].viewX = 0;

    if (players[0].viewX + players[0].viewW >= LW * TW)
      players[0].viewX = (LW * TW) - players[0].viewW;

    if (players[0].viewY < 0) players[0].viewY = 0;

    if (players[0].viewY + players[0].viewH >= LH * TH)
      players[0].viewY = (LH * TH) - players[0].viewH;


    // Set tile drawing dimensions
    src.w = TW;
    src.h = TH;
    src.x = 0;

    // If there is a sky, draw it
    if (bgPE->type == PE_SKY) {

      dst.x = 0;
      dst.w = screenW;
      dst.h = bgScale;

      dst.y = 0;
      SDL_FillRect(screen, &dst, 156);

      for (y = ((int)(players[0].viewY) % bgScale);
           y < players[0].viewH; y += bgScale    ) {

        dst.y = y;
        SDL_FillRect(screen, &dst, 157 + (y / bgScale));

      }

      // Assign the correct portion of the sky palette
      bgPE->position = players[0].viewY + (players[0].viewH / 2) - 4;

      // Show sun / moon / etc.
      if (skyOrb) {

        dst.x = players[0].viewW * 0.8f;
        dst.y = players[0].viewH * 0.12f;
        src.y = skyOrb << 5;
        SDL_BlitSurface(tileSet, &src, screen, &dst);

      }

    } else {

      // If there is no sky, draw a blank background
      // This is only very occasionally actually visible
      SDL_FillRect(screen, NULL, 127);

    }

    // Tell the diagonal lines background where it should be
    if (bgPE->type == PE_1D) {

      bgPE->position = players[0].viewX + players[0].viewY;

    }

    // Tell the parallaxing background where it should be
    if (bgPE->type == PE_2D) {

      ((short int *)&(bgPE->position))[0] = players[0].viewX;
      ((short int *)&(bgPE->position))[1] = players[0].viewY;

    }


    // Show background tiles

    for (y = 0; y <= ((players[0].viewH - 1) >> 5) + 1; y++) {

      for (x = 0; x <= ((players[0].viewW - 1) >> 5) + 1; x++) {

        // Get the grid element from the given coordinates
        ge = grid[y + ((int)players[0].viewY >> 5)] + x +
             ((int)players[0].viewX >> 5);

        // If this tile uses a black background, draw it
        if (ge->bg) {

          dst.x = (x << 5) - ((int)players[0].viewX & 31);
          dst.y = (y << 5) - ((int)players[0].viewY & 31);
          dst.w = dst.h = TW;
          SDL_FillRect(screen, &dst, 31);

        }

        // If this is not a foreground tile, draw it
        if ((ge->event < 123) || (ge->event > 125)) {

          dst.x = (x << 5) - ((int)players[0].viewX & 31);
          dst.y = (y << 5) - ((int)players[0].viewY & 31);
          src.y = ge->tile << 5;
          SDL_BlitSurface(tileSet, &src, screen, &dst);

        }

      }

    }


    // Show events
    for (y = 0; y <= ((players[0].viewH - 1) >> 5) + 1; y++) {

      for (x = 0; x <= ((players[0].viewW - 1) >> 5) + 1; x++)
        processEvent(x, y, frame);

    }


    // Show the player

    dst.x = players[0].x - players[0].viewX;
    dst.y = players[0].y + 35 - animSet[players[0].anim].sprites[frame %
                           animSet[players[0].anim].frames]->h
            - players[0].viewY;
    SDL_BlitSurface(animSet[players[0].anim].sprites[frame %
                    animSet[players[0].anim].frames], NULL, screen, &dst);


    // Show foreground tiles

    for (y = 0; y <= ((players[0].viewH - 1) >> 5) + 1; y++) {

      for (x = 0; x <= ((players[0].viewW - 1) >> 5) + 1; x++) {

        // Get the grid element from the given coordinates
        ge = grid[y + ((int)players[0].viewY >> 5)] + x +
             ((int)players[0].viewX >> 5);

        // If this is a foreground tile, draw it
        if ((ge->event >= 123) && (ge->event <= 125)) {

          dst.x = (x << 5) - ((int)players[0].viewX & 31);
          dst.y = (y << 5) - ((int)players[0].viewY & 31);
          src.y = ge->tile << 5;
          SDL_BlitSurface(tileSet, &src, screen, &dst);

        }

      }

    }


    // The panel
    // Design decision: When the width of the player's viewport is greater than
    // 320, the panel will not fill up the whole space. I decided that as the
    // game used the latin alphabet, and the latin alphabet is read from the
    // left, then the panel should appear to the left. Another option would have
    // been to have it in the centre, but this would obscure Jazz when he was at
    // the bottom of the level. As it is, Jazz is still obscured at viewport
    // widths between 321 and 672. A new approach may be needed, e.g. splitting
    // the panel down the middle.

    // Show panel

    if (players[0].viewW <= 320) players[0].viewH += 33;
    dst.x = 0;
    dst.y = players[0].viewH - 33;
    SDL_BlitSurface(panel, NULL, screen, &dst);
    dst.y += 32;
    dst.w = 320;
    dst.h = 1;
    SDL_FillRect(screen, &dst, 31);


    // Show panel data

    // Show score
    showNumber(players[0].score++, 84, players[0].viewH - 27, &panelSmallFont);

    // Show time remaining
    x = time - ticks;
    y = x / (60 * 1000);
    showNumber(y, 116, players[0].viewH - 27, &panelSmallFont);
    x -= (y * 60 * 1000);
    y = x / 1000;
    showNumber(y, 136, players[0].viewH - 27, &panelSmallFont);
    x -= (y * 1000);
    y = x / 100;
    showNumber(y, 148, players[0].viewH - 27, &panelSmallFont);

    // Show lives
    showNumber(players[0].lives, 124, players[0].viewH - 13, &panelSmallFont);

    // Show planet number
    if (world <= 41) // Main game levels
      showNumber((world % 3) + 1, 184, players[0].viewH - 13, &panelSmallFont);
    else if ((world >= 50) && (world <= 52)) // Christmas levels
      showNumber(world - 49, 184, players[0].viewH - 13, &panelSmallFont);
    else // New levels
      showNumber(world, 184, players[0].viewH - 13, &panelSmallFont);

    // Show level number
    showNumber(level + 1, 196, players[0].viewH - 13, &panelSmallFont);

    // Show ammo
    // Temp: At the moment, just show infinity
    showString(":;", 224, players[0].viewH - 13, &panelSmallFont);

    // Temp: FPS, screen resolution, displayed in the ammo type box
    showString("FPS", 252, players[0].viewH - 27, &panelBigFont);
    showNumber((short int)smoothfps, 308, players[0].viewH - 27, &panelBigFont);
    showNumber(screenW, 276, players[0].viewH - 15, &panelBigFont);
    showNumber(screenH, 308, players[0].viewH - 15, &panelBigFont);

    // Draw the health bar

    dst.x = 20;
    dst.y = players[0].viewH - 13;
    dst.h = 7;

    // For testing:
    players[0].energy = 4 - (((ticks + (10 * 60 * 1000) - time)  / 4000) % 5);

    if ((int)(players[0].energyBar) < (players[0].energy << 4)) {

      if ((players[0].energy << 4) - players[0].energyBar < spf * 64)
        players[0].energyBar = players[0].energy << 4;
      else players[0].energyBar += spf * 64;

    } else if ((int)(players[0].energyBar) > (players[0].energy << 4)) {

      if (players[0].energyBar - (players[0].energy << 4) < spf * 64)
        players[0].energyBar = players[0].energy << 4;
      else players[0].energyBar -= spf * 64;

    }

    if (players[0].energyBar > 1) {

      dst.w = players[0].energyBar - 1;

      // Choose energy bar colour
      if (players[0].energy == 4) x = 24;
      if (players[0].energy == 3) x = 17;
      if (players[0].energy == 2) x = 80;
      if (players[0].energy <= 1) x = 32 + ((frame * 4) & 15);

      // Draw energy bar
      SDL_FillRect(screen, &dst, x);

      dst.x += dst.w;
      dst.w = 64 - dst.w;

    } else dst.w = 64;

    // Fill in remaining energy bar space with black
    SDL_FillRect(screen, &dst, 31);


    // Temp: Can skip to the next level by pressing the spacebar
    if ((ticks > time) || (keys[K_FIRE].state == SDL_PRESSED)) {

      keys[K_FIRE].state = SDL_RELEASED;

      if (nextlevel == 99)
        DORETURN(loadScene(sceneFile), freeLevel();)

      freeLevel();

      if (nextlevel == 99)
        sceneLoop();

      DORETURN(loadNextLevel(), )

    }

  }

  return;

}




