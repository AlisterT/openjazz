
/*
 *
 * levelload.c
 * Created on the 22nd of July 2008 from parts of level.c
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
 * Deals with the loading of ordinary levels.
 *
 */


#include "level.h"
#include <string.h>


int loadSprites (char * fn) {

  FILE *f, *mf, *sf;
  unsigned char *pixels, *sorted;
  int mposition, sposition;
  int i, x, y, w, h, m; // VERY VERY BAD VARIABLE NAMES
                        // But cleaning up this little coding faux-pas would
                        // take ages.

  // Open fn
  sf = fopenFromPath(fn);

  if (sf == NULL) return FAILURE;

  // This function loads all the sprites, not fust those in fn
  // Note: Lower case is necessary for Unix support
  mf = fopenFromPath("mainchar.000");

  if (mf == NULL) {

    fclose(sf);

    return FAILURE;

  }

  sprites = fgetc(sf);
  sprites += fgetc(sf) << 8; // Not that the sprite set can ever be larger than
                             // 256...

  // Include space in the sprite set for the blank sprite at the end
  spriteSet = malloc(sizeof(sprite) * (sprites + 1));

  // Read horizontal offsets
  for (i = 0; i < sprites; i++) spriteSet[i].x = fgetc(sf) << 2;

  // Read vertical offsets
  for (i = 0; i < sprites; i++) spriteSet[i].y = fgetc(sf);

  // Find where the sprites start in fn
  sposition = ftell(sf);

  // Find where the sprites start in mainchar.000
  mposition = 2;

  // Loop through all the sprites to be loaded
  for (i = 0; i < sprites; i++) {

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
      spriteSet[i].pixels = createBlankSurface();
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
    w += fgetc(f) << 10;

    // Height
    h = fgetc(f);
    h += fgetc(f) << 8;

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
    m += fgetc(f) << 8;

    // Allocate space for pixel data
    pixels = malloc(w * h);

    // Allocate space for descrambling
    sorted = malloc(w * h);

    // Actually, m is for mask offset.
    // Sprites can be either masked or not masked.
    if (!m) {

      // Not masked
      // Load the pixel data directly for descrambling

      fseek(f, 2, SEEK_CUR);

      fread(pixels, w, h, f);


    } else {

      // Masked
      // Load the pixel data according to the mask

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

          if (!(x & 3)) m = fgetc(f);
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
    spriteSet[i].pixels = createSurface(sorted, w, h);
    SDL_SetColorKey(spriteSet[i].pixels, SDL_SRCCOLORKEY, SKEY);

    // Free redundant data
    free(pixels);

    // Check if the next sprite exists
    // If not, create blank sprites for the remainder
    fseek(sf, sposition, SEEK_SET);

    if (fgetc(sf) == -1) {

      for (i++; i < sprites; i++) {

        spriteSet[i].pixels = createBlankSurface();

      }

    }

  }

  fclose(mf);
  fclose(sf);


  // Include a blank sprite at the end

  spriteSet[sprites].pixels = createBlankSurface();
  spriteSet[sprites].x = 0;
  spriteSet[sprites].y = 0;

  return SUCCESS;

}


int loadTiles (char * fn) {

  FILE *f;
  unsigned char *buffer;
  int rle, pos, index, count;
  int nTiles;


  f = fopenFromPath(fn);

  if (f == NULL) return 0;


  // Load the palette

  loadPalette(levelPalette, f);

  // Apply the palette to surfaces that already exist, e.g. fonts
  usePalette(levelPalette);


  // Load the background palette

  loadPalette(skyPalette, f);


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

    } else if (!rle) { // This happens at the end of each tile

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
  char *subFN;
  unsigned char *buffer;
  paletteEffect *pe;
  unsigned char birdEvent[ELENGTH] = {0, 0, 0, 0, 8, 51, 52, 0,
                                      0, 0, 7, 0, 0, B_BLASTER, 1, 1,
                                      0, 10, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char nTiles;
  int count, x, y;


  f = fopenFromPath(fn);

  if (f == NULL) return FAILURE;


  // Load level data from a Level#.### file

  subFN = malloc(12);

  // Feline says this byte gives the correct tileset extension. All I get is
  // the level extension.
  fseek(f, -15, SEEK_END);
  x = fgetc(f) ^ 4;

  // Load tile set from corresponding blocks.###
  // Note: Lower case is required for Unix support

  // Boss levels are special cases
  if (x == 18) nTiles = loadTiles("blocks.002");
  else if (x == 19) nTiles = loadTiles("blocks.005");
  else if (x == 20) nTiles = loadTiles("blocks.008");
  else if (x == 21) nTiles = loadTiles("blocks.011");
  else if (x == 22) nTiles = loadTiles("blocks.014");
  else if (x == 39) nTiles = loadTiles("blocks.032");
  else if (x == 40) nTiles = loadTiles("blocks.035");
  else if (x == 41) nTiles = loadTiles("blocks.038");
  else {

    // Construct and use file name for non-boss levels
    sprintf(subFN, "blocks.%3s", fn + 7);
    nTiles = loadTiles(subFN);

  }

  if (!nTiles) {

    free(subFN);
    fclose(f);

    return FAILURE;

  }

  // Load sprite set from corresponding Sprites.###
  // Note: Lower case is required for Unix support
  sprintf(subFN, "sprites.%3s", fn + 7);

  if (loadSprites(subFN)) {

    free(subFN);
    SDL_FreeSurface(tileSet);
    fclose(f);

    return FAILURE;

  }

  free(subFN);

  // Skip to tile and event reference data
  fseek(f, 39, SEEK_SET);

  // Load tile and event references

  buffer = loadRLE(f, LW * LH * 2);

  // Create grid from data
  for (x = 0; x < LW; x++) {

    for (y = 0; y < LH; y++) {

      grid[y][x].tile = buffer[(y + (x * LH)) << 1];
      grid[y][x].bg = buffer[((y + (x * LH)) << 1) + 1] >> 7;
      grid[y][x].event = buffer[((y + (x * LH)) << 1) + 1] & 127;
      grid[y][x].hits = 0;

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

  for (count = 0; count < nTiles; count++) {

    for (y = 0; y < TH; y++) {

      for (x = 0; x < TW; x++) {

        if (mask[count][((y / 4) * 8) + (x / 4)] == 1)
          ((char *)(tileSet->pixels))[(count * 32 * 32) + (y * 32) + x] = 88;

      }

    }

  }

  if (SDL_MUSTLOCK(tileSet)) SDL_UnlockSurface(tileSet);*/


  // Load special event path

  count = fgetc(f);
  count += fgetc(f) << 8;
  fseek(f, -2, SEEK_CUR);
  buffer = loadRLE(f, count);
  pathLength = buffer[0] + (buffer[1] << 8);
  pathNode = 0;
  pathX = malloc(pathLength);
  pathY = malloc(pathLength);

  for (count = 0; count < pathLength; count++) {
    pathX[count] = ((signed char *)buffer)[(count * 2) + 3];
    pathY[count] = ((signed char *)buffer)[(count * 2) + 2];
  }

  free(buffer);


  // Load event set

  buffer = loadRLE(f, EVENTS * ELENGTH);

  // Set event 0
  memset(*eventSet, 0, ELENGTH);

  for (count = 1; count < EVENTS; count++) {

    memcpy(eventSet[count], buffer + (count * ELENGTH), ELENGTH);
    eventSet[count][E_MOVEMENTSP]++;

  }

  free(buffer);

  // Create the bird
  memcpy(eventSet[121], birdEvent, ELENGTH);


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
      if (x > sprites) x = sprites;

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
  buffer = loadString(f);
  loadMusic(buffer);

  // 25 bytes of undiscovered usefulness, less the music file name
  fseek(f, 25 - strlen(buffer), SEEK_CUR);
  free(buffer);

  // End of episode cutscene
  sceneFile = loadString(f);

  // 51 bytes of undiscovered usefulness, less the cutscene file name
  fseek(f, 51 - strlen(sceneFile), SEEK_CUR);

  // Finally, some data I know how to use!

  // First up, the player's coordinates
  // These are grid coordinates, so they must be shifted into real coordinates
  checkX = fgetc(f);
  checkX += fgetc(f) << 8;
  checkY = fgetc(f);
  checkY += fgetc(f) << 8;
  localPlayer->x = checkX << 15;
  localPlayer->y = checkY << 15;

  // The level being loaded is now the current level
  if (fn != currentLevel) {

    free(currentLevel);
    currentLevel = malloc(strlen(fn) + 1);
    strcpy(currentLevel, fn);

  }

  // Next level
  free(nextLevel);
  x = fgetc(f);
  y = fgetc(f);

  if (x != 99) {

    nextLevel = malloc(11);
    sprintf(nextLevel, "level%1i.%03i", x, y);

  } else {

    nextLevel = malloc(8);
    sprintf(nextLevel, "endepis"); // Just like Jazz 2

  }

  // Thanks to Feline and the JCS94 team for the next bits:

  // 9 more bytes left in "Section 14"
  fseek(f, 9, SEEK_CUR);

  // Now at "Section 15"


  // Load player's animation set references

  buffer = loadRLE(f, PANIMS * 2);

  for (count = 0; count < PANIMS; count++)
    localPlayer->anims[count] = buffer[count << 1];

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
      bgPE->speed = FH;
      bgPE->position = 0;

      break;

    case 8:

      // Parallaxing background effect
      bgPE = malloc(sizeof(paletteEffect));
      bgPE->next = NULL;
      bgPE->first = 128;
      bgPE->amount = 64;
      bgPE->type = PE_2D;
      bgPE->speed = FE;
      bgPE->position = 0;

      break;

    case 9:

      // Diagonal stripes "parallaxing" background effect
      bgPE = malloc(sizeof(paletteEffect));
      bgPE->next = NULL;
      bgPE->first = 128;
      bgPE->amount = 32;
      bgPE->type = PE_1D;
      bgPE->speed = FH;
      bgPE->position = 0;

      break;

    default:

      // No effect, but bgPE must exist so here is a dummy animation
      bgPE = malloc(sizeof(paletteEffect));
      bgPE->next = NULL;
      bgPE->first = 255;
      bgPE->amount = 1;
      bgPE->type = PE_ROTATE;
      bgPE->speed = F1;
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
  pe->speed = F32;
  pe->position = 0;
  firstPE = pe;

  // In Diamondus: The waterfall palette animation
  pe = malloc(sizeof(paletteEffect));
  pe->next = firstPE;
  pe->first = 116;
  pe->amount = 8;
  pe->type = PE_ROTATE;
  pe->speed = F16;
  pe->position = 0;
  firstPE = pe;

  // The following were discoverd by Unknown/Violet

  pe = malloc(sizeof(paletteEffect));
  pe->next = firstPE;
  pe->first = 124;
  pe->amount = 3;
  pe->type = PE_ROTATE;
  pe->speed = F16;
  pe->position = 0;
  firstPE = pe;

  if ((bgPE->type != PE_1D) && (bgPE->type != PE_2D)) {

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 132;
    pe->amount = 8;
    pe->type = PE_ROTATE;
    pe->speed = F16;
    pe->position = 0;
    firstPE = pe;

  }

  if ((bgPE->type != PE_SKY) && (bgPE->type != PE_2D)) {

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 160;
    pe->amount = 32;
    pe->type = PE_ROTATE;
    pe->speed = -F16;
    pe->position = 0;
    firstPE = pe;

  }

  if (bgPE->type != PE_SKY) {

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 192;
    pe->amount = 32;
    pe->type = PE_ROTATE;
    pe->speed = F32;
    pe->position = 0;
    firstPE = pe;

    pe = malloc(sizeof(paletteEffect));
    pe->next = firstPE;
    pe->first = 224;
    pe->amount = 16;
    pe->type = PE_ROTATE;
    pe->speed = F16;
    pe->position = 0;
    firstPE = pe;

  }


  // Some initial values for the player

  localPlayer->facing = 1;
  localPlayer->dx = 0;
  localPlayer->dy = 0;
  localPlayer->jumpHeight = 92 * F1;
  localPlayer->jumpY = 65 * F32;
  localPlayer->energy = 4;
  localPlayer->energyBar = 0;
  localPlayer->reaction = 0;
  localPlayer->reactionTime = 0;
  localPlayer->floating = 0;
  localPlayer->event = 0;


  scalePalette(fontmn1->pixels, -F2, (16 * -2) + 240);


  // Set the tick at which the level will end
  endTicks = (5 - difficulty) * 2 * 60 * 1000;


  firstBullet = NULL;
  unusedBullet = NULL;
  firstEvent = NULL;
  unusedEvent = NULL;

  stats = S_NONE;

  return SUCCESS;

}



