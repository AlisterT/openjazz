
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


int loadSprites (char * fn) {

  FILE *f, *mf, *sf;
  unsigned char *pixels, *sorted;
  int mposition, sposition;
  int i, x, y, w, h, m; // VERY VERY BAD VARIABLE NAMES
                        // But cleaning up this little coding faux-pas would
                        // take ages.

  // Open fn
  sf = fopenFromPath(fn);

  // This function loads all the sprites, not fust those in fn
  // Note: Lower case is necessary for Unix support
  mf = fopenFromPath("mainchar.000");

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
      spriteSet[i].pixels = createBlankSurface(levelPalette);
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
    spriteSet[i].pixels = createSurface(sorted, levelPalette, w, h);
    SDL_SetColorKey(spriteSet[i].pixels, SDL_SRCCOLORKEY, SKEY);

    // Free redundant data
    free(pixels);

    // Check if the next sprite exists
    // If not, create blank sprites for the remainder
    fseek(sf, sposition, SEEK_SET);

    if (fgetc(sf) == -1) {

      for (i++; i < sprites; i++) {

        spriteSet[i].pixels = createBlankSurface(levelPalette);

      }

    }

  }

  fclose(mf);
  fclose(sf);


  // Include a blank sprite at the end

  spriteSet[sprites].pixels = createBlankSurface(levelPalette);
  spriteSet[sprites].x = 0;
  spriteSet[sprites].y = 0;

  return CONTINUE;

}


int loadTiles (char * fn) {

  FILE *f;
  unsigned char *buffer;
  int rle, pos, index, count;
  int nTiles;


  f = fopenFromPath(fn);


  // Load the palette

  loadPalette(levelPalette, f);

  // Apply the palette to surfaces that already exist, e.g. fonts
  usePalette(levelPalette);


  // Load the background palette

  loadPalette(levelBGPalette, f);


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

  tileSet = createSurface(buffer, levelPalette, TW, TH * nTiles);
  SDL_SetColorKey(tileSet, SDL_SRCCOLORKEY, TKEY);

  return nTiles;

}



int loadLevel (char * fn) {

  FILE *f;
  unsigned char *buffer;
  paletteEffect *pe;
  unsigned char birdEvent[ELENGTH] = {0, 0, 0, 0, 8, 51, 52, 0,
                                      0, 0, 7, 0, 0, B_BLASTER, 1, 1,
                                      0, 10, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char nTiles;
  int count, x, y;


  f = fopenFromPath(fn);

  if (!strncasecmp(fn + 6, ".j1l", 4)) {

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
    loadSprites(subFN);

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

  // Set event 0
  memset(*eventSet, 0, ELENGTH);

  for (count = 1; count < EVENTS; count++)
    memcpy(eventSet[count], buffer + (count * ELENGTH), ELENGTH);

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
  x = fgetc(f);
  buffer = malloc(x + 1);
  for (y = 0; y < x; y++) buffer[y] = fgetc(f);
  buffer[x] = 0;
  loadMusic(buffer); // It's not as if this even does anything yet
  free(buffer);

  // 25 bytes of undiscovered usefulness, less the music file name
  fseek(f, 25 - x, SEEK_CUR);

  x = fgetc(f);
  if (!x) x = 9;
  sceneFile = malloc(x + 1);
  for (y = 0; y < x; y++) sceneFile[y] = fgetc(f);
  sceneFile[x] = 0;

  // 51 bytes of undiscovered usefulness, less the cutscene file name
  fseek(f, 51 - x, SEEK_CUR);

  // Finally, some data I know how to use!

  // First up, the player's coordinates
  // These are grid coordiantes, so they must be shifted into real coordinates
  checkX = fgetc(f);
  checkX += fgetc(f) << 8;
  checkY = fgetc(f);
  checkY += fgetc(f) << 8;
  localPlayer->x = checkX << 5;
  localPlayer->y = checkY << 5;

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


  // Some initial values for the player

  localPlayer->facing = 1;
  localPlayer->dx = 0;
  localPlayer->dy = 0;
  localPlayer->jumpDuration = 300;
  localPlayer->jumpTime = 0;
  localPlayer->energy = 4;
  localPlayer->energyBar = 0;
  localPlayer->reaction = 0;
  localPlayer->reactionTime = 0;
  localPlayer->floating = 0;


  // Set the time at which the level will end
  time = SDL_GetTicks() + ((5 - difficulty) * 2 * 60 * 1000);


  return CONTINUE;

}


int loadNextLevel () {

  FILE *f;
  char *fn;
  int count;

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

  fn = malloc(11);

  // Load whichever level nextlevel and nextworld refer to
  sprintf(fn, "level%1d.%03d", level, world);
  f = fopenFromPath(fn);

  // If that didn't work, scan for level
  for (count = 0; (count < 156) && !f; count++) {

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

  if (f) {

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
  while (firstPE) {

    pe = firstPE->next;
    free(firstPE);
    firstPE = pe;

  }

  freeBullets();
  freeEvents();

  SDL_FreeSurface(tileSet);

  for (count = 0; count <= sprites; count++)
    SDL_FreeSurface(spriteSet[count].pixels);

  free(spriteSet);

  free(sceneFile);

  freeMusic();

  return;

}



int checkMask (int x, int y) {

  // Anything off the edge of the map is solid
  if ((x < 0) || (y < 0) || (x > LW * TW) || (y > LH * TH)) return 1;

  // Check the mask in the tile in question
  return mask[grid[y >> 5][x >> 5].tile]
             [(((y & 31) >> 2) << 3) + ((x & 31) >> 2)];

}


void levelLoop (void) {

  gridElement *ge;
  bullet *bul, *prevBul;
  event *evt, *nextEvt;
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

    // Determine the player's trajectory

    if (keys[K_LEFT].state == SDL_PRESSED) {

      if (localPlayer->dx > 0) localPlayer->dx -= 600 * spf;
      if (localPlayer->dx > -PS_WALK) localPlayer->dx -= 300 * spf;
      if (localPlayer->dx > -PS_RUN) localPlayer->dx -= 150 * spf;
      if (localPlayer->dx < -PS_RUN) localPlayer->dx = -PS_RUN;

      localPlayer->facing = 0;

    } else if (keys[K_RIGHT].state == SDL_PRESSED) {
    
      if (localPlayer->dx < 0) localPlayer->dx += 600 * spf;
      if (localPlayer->dx < PS_WALK) localPlayer->dx += 300 * spf;
      if (localPlayer->dx < PS_RUN) localPlayer->dx += 150 * spf;
      if (localPlayer->dx > PS_RUN) localPlayer->dx = PS_RUN;

      localPlayer->facing = 1;

    } else {

      if (localPlayer->dx > 0) {

        if (localPlayer->dx < 1000 * spf) localPlayer->dx = 0;
        else localPlayer->dx -= 1000 * spf;

      }

      if (localPlayer->dx < 0) {

        if (localPlayer->dx > -1000 * spf) localPlayer->dx = 0;
        else localPlayer->dx += 1000 * spf;

      }

    }


    if (localPlayer->floating) {
    
      if (keys[K_UP].state == SDL_PRESSED) {

        if (localPlayer->dy > 0) localPlayer->dy -= 600 * spf;
        if (localPlayer->dy > -PS_WALK) localPlayer->dy -= 300 * spf;
        if (localPlayer->dy > -PS_RUN) localPlayer->dy -= 150 * spf;
        if (localPlayer->dy < -PS_RUN) localPlayer->dy = -PS_RUN;

        localPlayer->facing = 0;

      } else if (keys[K_DOWN].state == SDL_PRESSED) {
    
        if (localPlayer->dy < 0) localPlayer->dy += 600 * spf;
        if (localPlayer->dy < PS_WALK) localPlayer->dy += 300 * spf;
        if (localPlayer->dy < PS_RUN) localPlayer->dy += 150 * spf;
        if (localPlayer->dy > PS_RUN) localPlayer->dy = PS_RUN;

        localPlayer->facing = 1;

      } else {

        if (localPlayer->dy > 0) {

          if (localPlayer->dy < 1000 * spf) localPlayer->dy = 0;
          else localPlayer->dy -= 1000 * spf;

        }

        if (localPlayer->dy < 0) {

          if (localPlayer->dy > -1000 * spf) localPlayer->dy = 0;
          else localPlayer->dy += 1000 * spf;

        }

      }

    } else {

      if (keys[K_JUMP].state == SDL_PRESSED) {

        if (localPlayer->jumpTime > ticks) {

          localPlayer->dy = PS_JUMP;

        } else if ((checkMask(localPlayer->x + 12, localPlayer->y + 1) ||
                    checkMask(localPlayer->x + 16, localPlayer->y + 1) ||
                    checkMask(localPlayer->x + 20, localPlayer->y + 1)   ) &&
                   (!checkMask(localPlayer->x + 16, localPlayer->y - 33) ||
                    (grid[(int)(localPlayer->y - 21) >> 5]
                         [(int)(localPlayer->x + 16) >> 5].event == 122)   ) ) {

          localPlayer->jumpTime = ticks + localPlayer->jumpDuration;
          localPlayer->dy = PS_JUMP;

        }

      }

      if (keys[K_JUMP].state == SDL_RELEASED) localPlayer->jumpTime = 0;

      if (checkMask(localPlayer->x + 16, localPlayer->y - 21) &&
          (grid[(int)(localPlayer->y - 21) >> 5]
               [(int)(localPlayer->x + 16) >> 5].event != 122)  ) {

        localPlayer->jumpTime = 0;
        localPlayer->dy = 0;

      }
    
      localPlayer->dy += 3000 * spf;
      if (localPlayer->dy > PS_FALL) localPlayer->dy = PS_FALL;

    }

    // Apply as much of that trajectory as possible, without going into the
    // scenery
    
    dx = localPlayer->dx * spf;
    dy = localPlayer->dy * spf;

    x = (int)dx;
    y = (int)dy;

    // First for the vertical component of the trajectory

    while (y > 0) {

      if (checkMask(localPlayer->x + 12, localPlayer->y + 1) ||
          checkMask(localPlayer->x + 16, localPlayer->y + 1) ||
          checkMask(localPlayer->x + 20, localPlayer->y + 1)   ) break;

      localPlayer->y++;
      y--;

    }


    while (y < 0) {

      if (checkMask(localPlayer->x + 16, localPlayer->y - 21) &&
          (grid[(int)(localPlayer->y - 21) >> 5]
               [(int)(localPlayer->x + 16) >> 5].event != 122)  ) break;

      localPlayer->y--;
      y++;

    }

    dy -= (int)dy;

    if (((dy > 0) &&
        !(checkMask(localPlayer->x + 12, localPlayer->y + dy) ||
          checkMask(localPlayer->x + 16, localPlayer->y + dy) ||
          checkMask(localPlayer->x + 20, localPlayer->y + dy)   )) ||
        ((dy < 0) &&
         (!checkMask(localPlayer->x + 16, localPlayer->y + dy - 20) ||
          (grid[(int)(localPlayer->y + dy - 20) >> 5]
               [(int)(localPlayer->x + 16) >> 5].event == 122)        )))
      localPlayer->y += dy;

    // Then for the horizontal component of the trajectory

    while (x < 0) {

      if (checkMask(localPlayer->x + 5, localPlayer->y - 19)) break;

      localPlayer->x--;
      x++;

    }

    while (x > 0) {

      if (checkMask(localPlayer->x + 27, localPlayer->y - 19)) break;

      localPlayer->x++;
      x--;

    }

    dx -= (int)dx;

    if (((dx < 0) &&
         !checkMask(localPlayer->x + 6 + dx, localPlayer->y - 19)) ||
        ((dx > 0) &&
         !checkMask(localPlayer->x + 26 + dx, localPlayer->y - 19))  )
      localPlayer->x += dx;

    // If on an uphill slope, push the player upwards
    while (checkMask(localPlayer->x + 16, localPlayer->y))
      localPlayer->y--;


    // Choose animation

    if (localPlayer->reaction == PR_KILLED)
      localPlayer->anim = localPlayer->anims[PA_LDIE + localPlayer->facing];

    else if (localPlayer->floating)
      localPlayer->anim = localPlayer->anims[PA_LBOARD + localPlayer->facing];

    else if (localPlayer->dy >= 0) {

      if (checkMask(localPlayer->x + 12, localPlayer->y + 4) ||
          checkMask(localPlayer->x + 16, localPlayer->y + 4) ||
          checkMask(localPlayer->x + 20, localPlayer->y + 4) ||
          checkMask(localPlayer->x + 12, localPlayer->y + 12) ||
          checkMask(localPlayer->x + 16, localPlayer->y + 12) ||
          checkMask(localPlayer->x + 20, localPlayer->y + 12)   ) {

        if (localPlayer->dx) {

          if (localPlayer->dx <= -PS_RUN)
            localPlayer->anim = localPlayer->anims[PA_LRUN];
          else if (localPlayer->dx >= PS_RUN)
            localPlayer->anim = localPlayer->anims[PA_RRUN];
          else if ((localPlayer->dx < 0) && (localPlayer->facing == 1))
            localPlayer->anim = localPlayer->anims[PA_LSTOP];
          else if ((localPlayer->dx > 0) && !localPlayer->facing)
            localPlayer->anim = localPlayer->anims[PA_RSTOP];
          else
            localPlayer->anim = localPlayer->anims[PA_LWALK +
                                                   localPlayer->facing];

        } else {

          if (!checkMask(localPlayer->x + 12, localPlayer->y + 12) &&
              !checkMask(localPlayer->x + 8, localPlayer->y + 8))
            localPlayer->anim = localPlayer->anims[PA_LEDGE];
          else if (!checkMask(localPlayer->x + 20, localPlayer->y + 12) &&
                   !checkMask(localPlayer->x + 24, localPlayer->y + 8))
            localPlayer->anim = localPlayer->anims[PA_REDGE];
          else if (keys[K_FIRE].state == SDL_PRESSED)
            localPlayer->anim = localPlayer->anims[PA_LSHOOT +
                                                   localPlayer->facing];
          else
            localPlayer->anim = localPlayer->anims[PA_LSTAND +
                                                 localPlayer->facing];

        }

      } else localPlayer->anim = localPlayer->anims[PA_LFALL +
                                                  localPlayer->facing];

    } else if (localPlayer->dy >= PS_JUMP)
      localPlayer->anim = localPlayer->anims[PA_LJUMP + localPlayer->facing];

    else
      localPlayer->anim = localPlayer->anims[PA_LSPRING - localPlayer->facing];


    // Calculate viewport dimensions

    // Can we can see below the panel?
    if (localPlayer->viewW > panel->w) localPlayer->viewH = screenH;
    else localPlayer->viewH = screenH - 33;
    localPlayer->viewW = screenW;

    localPlayer->viewX = localPlayer->x + 8 - (localPlayer->viewW / 2);
    localPlayer->viewY = localPlayer->y - 24 - (localPlayer->viewH / 2);

    if (localPlayer->viewX < 0) localPlayer->viewX = 0;

    if (localPlayer->viewX + localPlayer->viewW >= LW * TW)
      localPlayer->viewX = (LW * TW) - localPlayer->viewW;

    if (localPlayer->viewY < 0) localPlayer->viewY = 0;

    if (localPlayer->viewY + localPlayer->viewH >= LH * TH)
      localPlayer->viewY = (LH * TH) - localPlayer->viewH;


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

      for (y = ((int)(localPlayer->viewY) % bgScale);
           y < localPlayer->viewH; y += bgScale    ) {

        dst.y = y;
        SDL_FillRect(screen, &dst, 157 + (y / bgScale));

      }

      // Assign the correct portion of the sky palette
      bgPE->position = localPlayer->viewY + (localPlayer->viewH / 2) - 4;

      // Show sun / moon / etc.
      if (skyOrb) {

        dst.x = localPlayer->viewW * 0.8f;
        dst.y = localPlayer->viewH * 0.12f;
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

      bgPE->position = localPlayer->viewX + localPlayer->viewY;

    }

    // Tell the parallaxing background where it should be
    if (bgPE->type == PE_2D) {

      ((short int *)&(bgPE->position))[0] = localPlayer->viewX;
      ((short int *)&(bgPE->position))[1] = localPlayer->viewY;

    }


    // Show background tiles

    for (y = 0; y <= ((localPlayer->viewH - 1) >> 5) + 1; y++) {

      for (x = 0; x <= ((localPlayer->viewW - 1) >> 5) + 1; x++) {

        // Get the grid element from the given coordinates
        ge = grid[y + ((int)localPlayer->viewY >> 5)] + x +
             ((int)localPlayer->viewX >> 5);

        // If this tile uses a black background, draw it
        if (ge->bg) {

          dst.x = (x << 5) - ((int)localPlayer->viewX & 31);
          dst.y = (y << 5) - ((int)localPlayer->viewY & 31);
          dst.w = dst.h = TW;
          SDL_FillRect(screen, &dst, 31);

        }

        // If this is not a foreground tile, draw it
        if ((eventSet[ge->event][E_BEHAVIOUR] != 38) &&
            ((ge->event < 124) || (ge->event > 125))  ) {

          dst.x = (x << 5) - ((int)localPlayer->viewX & 31);
          dst.y = (y << 5) - ((int)localPlayer->viewY & 31);
          src.y = ge->tile << 5;
          SDL_BlitSurface(tileSet, &src, screen, &dst);

        }

      }

    }

    // Search for active events
    for (y = ((int)localPlayer->viewY >> 5) - 5;
         y < ((int)(localPlayer->viewY + localPlayer->viewH) >> 5) + 5; y++) {

      for (x = ((int)localPlayer->viewX >> 5) - 5;
           x < ((int)(localPlayer->viewX + localPlayer->viewW) >> 5) + 5; x++) {

        if ((x >= 0) && (y >= 0) && (x < LW) && (y < LH) && grid[y][x].event) {

          dx = -1;
          evt = firstEvent;

          while (evt) {

            if ((evt->gridX == x) && (evt->gridY == y)) {

              dx = 0;

              break;

            }

            evt = evt->next;

          }

          if (dx) createEvent(x, y);

        }

      }

    }

    // Process active events
    evt = firstEvent;

    while (evt) {

      if ((evt->anim != E_FINISHANIM) &&
          (grid[evt->gridY][evt->gridX].event != 121) &&
          ((evt->x < localPlayer->viewX - 160) ||
           (evt->x > localPlayer->viewX + localPlayer->viewW + 160) ||
           (evt->y < localPlayer->viewY - 160) ||
           (evt->y > localPlayer->viewY + localPlayer->viewH + 160)   ) &&
          ((evt->gridX < ((int)localPlayer->viewX >> 5) - 1) ||
           (evt->gridX >
            ((int)(localPlayer->viewX + localPlayer->viewW) >> 5) + 1) ||
           (evt->gridY < ((int)localPlayer->viewY >> 5) - 1) ||
           (evt->gridY >
            ((int)(localPlayer->viewY + localPlayer->viewH) >> 5) + 1)   )     ) {

        // If the event and its origin are off-screen, and the event is not
        // in the process of self-destruction, remove it
        nextEvt = evt->next;
        removeEvent(evt);
        evt = nextEvt;

      } else {

        nextEvt = evt->next;
        processEvent(evt, ticks);
        evt = nextEvt;

      }

    }


    // Handle spikes
    if ((localPlayer->reaction == PR_NONE) &&
        (((grid[(int)(localPlayer->y - 20) >> 5]
               [(int)(localPlayer->x + 16) >> 5].event == 126) &&
          checkMask(localPlayer->x + 16, localPlayer->y - 21)  ) || /* Above */
         ((grid[(int)(localPlayer->y) >> 5]
               [(int)(localPlayer->x + 16) >> 5].event == 126) &&
          checkMask(localPlayer->x + 16, localPlayer->y + 1)  ) || /* Below */
         ((grid[(int)(localPlayer->y - 10) >> 5]
               [(int)(localPlayer->x + 6) >> 5].event == 126) &&
          checkMask(localPlayer->x + 5, localPlayer->y - 10)  ) || /* To left*/
         ((grid[(int)(localPlayer->y - 10) >> 5]
               [(int)(localPlayer->x + 26) >> 5].event == 126) && /* To right*/
          checkMask(localPlayer->x + 27, localPlayer->y - 10)  )   )) {

          localPlayer->energy--;

          if (localPlayer->energy) {

            localPlayer->reaction = PR_HURT;
            localPlayer->reactionTime = ticks + 2000;

            if (localPlayer->dx < 0) {

              localPlayer->dx = PS_RUN;
              localPlayer->dy = PS_JUMP;

            } else {

              localPlayer->dx = -PS_RUN;
              localPlayer->dy = PS_JUMP;

            }

          } else {

            localPlayer->reaction = PR_KILLED;
            localPlayer->reactionTime = ticks + 2000;

          }

        }


    // Show the player

    if (!((localPlayer->reaction == PR_HURT) && (frame & 1))) {

      dst.x = localPlayer->x - localPlayer->viewX +
              animSet[localPlayer->anim].
               sprites[frame % animSet[localPlayer->anim].frames].x;
      dst.y = localPlayer->y + 4 - localPlayer->viewY +
              animSet[localPlayer->anim].
               sprites[frame % animSet[localPlayer->anim].frames].y +
              animSet[localPlayer->anim].
               y[frame % animSet[localPlayer->anim].frames] -
              animSet[localPlayer->anim].sprites[0].pixels->h;
      SDL_BlitSurface(animSet[localPlayer->anim].
                       sprites[frame % animSet[localPlayer->anim].frames].
                       pixels, NULL, screen, &dst);

    }

    if (localPlayer->reaction == PR_INVINCIBLE) {

      dst.x = localPlayer->x + 6 - localPlayer->viewX;
      dst.y = localPlayer->y - 32 - localPlayer->viewY;
      SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                      NULL, screen, &dst);

      dst.x = localPlayer->x + 16 - localPlayer->viewX;
      dst.y = localPlayer->y - 32 - localPlayer->viewY;
      SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                      NULL, screen, &dst);

      dst.x = localPlayer->x + 6 - localPlayer->viewX;
      dst.y = localPlayer->y - 16 - localPlayer->viewY;
      SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                      NULL, screen, &dst);

      dst.x = localPlayer->x + 16 - localPlayer->viewX;
      dst.y = localPlayer->y - 16 - localPlayer->viewY;
      SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                      NULL, screen, &dst);

    }

    // Handle firing

    if (keys[K_FIRE].state == SDL_PRESSED) {

      keys[K_FIRE].state = SDL_RELEASED; // Fix: This is going to make rapid-
                                         // fire somewhat difficult

      createPlayerBullet(localPlayer, ticks);

    }


    // Process and show bullets

    bul = firstBullet;
    prevBul = NULL;

    while (bul) {

      if ((ticks > bul->time) ||
          (checkMask(bul->x, bul->y) && (bul->type != B_BOUNCER))) {

        bul = bul->next;

        // Hit an obstacle, destroy the bullet
        removeBullet(prevBul);

      } else {

        if (bul->type == B_BOUNCER) {

          if (checkMask(bul->x, bul->y - 4)) bul->dy = 0;
          else if (checkMask(bul->x, bul->y + 4)) bul->dy = -600;
          else if (checkMask(bul->x - 4, bul->y)) bul->dx = 500;
          else if (checkMask(bul->x + 4, bul->y)) bul->dx = -500;
          else bul->dy += 3000 * spf;

        }

        bul->x += bul->dx * spf;
        bul->y += bul->dy * spf;

        dst.x = bul->x - (spriteSet[bul->type].pixels->w >> 1) -
                localPlayer->viewX;
        dst.y = bul->y - (spriteSet[bul->type].pixels->h >> 1) -
                localPlayer->viewY;

        // Show the bullet
        SDL_BlitSurface(spriteSet[bul->type].pixels, NULL, screen, &dst);

        prevBul = bul;
        bul = bul->next;

      }

    }


    // Show foreground tiles

    for (y = 0; y <= ((localPlayer->viewH - 1) >> 5) + 1; y++) {

      for (x = 0; x <= ((localPlayer->viewW - 1) >> 5) + 1; x++) {

        // Get the grid element from the given coordinates
        ge = grid[y + ((int)localPlayer->viewY >> 5)] + x +
             ((int)localPlayer->viewX >> 5);

        // If this is an "animated" foreground tile, draw it
        if (ge->event == 123) {

          dst.x = (x << 5) - ((int)localPlayer->viewX & 31);
          dst.y = (y << 5) - ((int)localPlayer->viewY & 31);
          if (frame & 1) src.y = eventSet[ge->event][E_YAXIS] << 5;
          else src.y = eventSet[ge->event][E_MULTIPURPOSE] << 5;
          SDL_BlitSurface(tileSet, &src, screen, &dst);

        }

        // If this is a foreground tile, draw it
        if ((ge->event == 124) || (ge->event == 125) ||
            (eventSet[ge->event][E_BEHAVIOUR] == 38)   ) {

          dst.x = (x << 5) - ((int)localPlayer->viewX & 31);
          dst.y = (y << 5) - ((int)localPlayer->viewY & 31);
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

    // Check for a change in ammo
    if (keys[K_CHANGE].state == SDL_PRESSED) {

      keys[K_CHANGE].state = SDL_RELEASED;

      localPlayer->ammoType = ((localPlayer->ammoType + 2) % 5) - 1;

      // If there is no ammo of this type, go to the next type that has ammo
      while ((localPlayer->ammoType > -1) &&
             !localPlayer->ammo[localPlayer->ammoType])
        localPlayer->ammoType = ((localPlayer->ammoType + 2) % 5) - 1;

    }

    // If the current ammo has been exhausted, go to the previous type that has
    // ammo
    while ((localPlayer->ammoType > -1) &&
           !localPlayer->ammo[localPlayer->ammoType])
      localPlayer->ammoType--;


    // Show panel

    // Change the ammo type display on the panel
    dst.x = 250;
    dst.y = 2;
    SDL_BlitSurface(panelAmmo[localPlayer->ammoType + 1], NULL, panel, &dst);

    if (localPlayer->viewW <= 320) localPlayer->viewH += 33;
    dst.x = 0;
    dst.y = localPlayer->viewH - 33;
    SDL_BlitSurface(panel, NULL, screen, &dst);
    dst.y += 32;
    dst.w = 320;
    dst.h = 1;
    SDL_FillRect(screen, &dst, 31);


    // Show panel data

    // Show score
    showNumber(localPlayer->score, 84, localPlayer->viewH - 27,
               &panelSmallFont);

    // Show time remaining
    x = time - ticks;
    y = x / (60 * 1000);
    showNumber(y, 116, localPlayer->viewH - 27, &panelSmallFont);
    x -= (y * 60 * 1000);
    y = x / 1000;
    showNumber(y, 136, localPlayer->viewH - 27, &panelSmallFont);
    x -= (y * 1000);
    y = x / 100;
    showNumber(y, 148, localPlayer->viewH - 27, &panelSmallFont);

    // Show lives
    showNumber(localPlayer->lives, 124, localPlayer->viewH - 13, &panelSmallFont);

    // Show planet number
    if (world <= 41) // Main game levels
      showNumber((world % 3) + 1, 184, localPlayer->viewH - 13, &panelSmallFont);
    else if ((world >= 50) && (world <= 52)) // Christmas levels
      showNumber(world - 49, 184, localPlayer->viewH - 13, &panelSmallFont);
    else // New levels
      showNumber(world, 184, localPlayer->viewH - 13, &panelSmallFont);

    // Show level number
    showNumber(level + 1, 196, localPlayer->viewH - 13, &panelSmallFont);

    // Show ammo
    if (localPlayer->ammoType == -1)
      showString(":;", 224, localPlayer->viewH - 13, &panelSmallFont);
    else showNumber(localPlayer->ammo[localPlayer->ammoType], 244,
                    localPlayer->viewH - 13, &panelSmallFont);

    // Draw the health bar

    dst.x = 20;
    dst.y = localPlayer->viewH - 13;
    dst.h = 7;

    if ((int)(localPlayer->energyBar) < (localPlayer->energy << 4)) {

      if ((localPlayer->energy << 4) - localPlayer->energyBar < spf * 64)
        localPlayer->energyBar = localPlayer->energy << 4;
      else localPlayer->energyBar += spf * 64;

    } else if ((int)(localPlayer->energyBar) > (localPlayer->energy << 4)) {

      if (localPlayer->energyBar - (localPlayer->energy << 4) < spf * 64)
        localPlayer->energyBar = localPlayer->energy << 4;
      else localPlayer->energyBar -= spf * 64;

    }

    if (localPlayer->energyBar > 1) {

      dst.w = localPlayer->energyBar - 1;

      // Choose energy bar colour
      if (localPlayer->energy == 4) x = 24;
      if (localPlayer->energy == 3) x = 17;
      if (localPlayer->energy == 2) x = 80;
      if (localPlayer->energy <= 1) x = 32 + ((frame * 4) & 15);

      // Draw energy bar
      SDL_FillRect(screen, &dst, x);

      dst.x += dst.w;
      dst.w = 64 - dst.w;

    } else dst.w = 64;

    // Fill in remaining energy bar space with black
    SDL_FillRect(screen, &dst, 31);


    // Temp: FPS and screen resolution
    showString("FPS", 252, 27, &panelBigFont);
    showNumber((short int)smoothfps, 308, 27, &panelBigFont);
    showNumber(screenW, 276, 15, &panelBigFont);
    showNumber(screenH, 308, 15, &panelBigFont);


    // Check if time has run out
    if (ticks > time) {

      localPlayer->reaction = PR_KILLED;
      localPlayer->reactionTime = ticks + 2000;

    }


    // Handle player reactions
    if (localPlayer->reaction && (localPlayer->reactionTime < ticks)) {

      switch (localPlayer->reaction) {

        case PR_KILLED:

          nextlevel = level;
          nextworld = world;
          localPlayer->lives--;

          freeLevel();

          x = checkX;
          y = checkY;

          DORETURN(loadNextLevel(), )

          localPlayer->x = x << 5;
          localPlayer->y = y << 5;

          break;

        case PR_WON:

          if (nextlevel == 99) DORETURN(loadScene(sceneFile), freeLevel();)

          freeLevel();

          if (nextlevel == 99) {

            sceneLoop();
            freeScene();

          }

          DORETURN(loadNextLevel(), )

          break;

      }

      localPlayer->reaction = PR_NONE;

    }

  }

  return;

}




