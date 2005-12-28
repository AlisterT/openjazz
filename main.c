
/*
 *
 * main.c
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
 * Contains the main function and core utility functions.
 *
 */


#define Main
#include "OpenJazz.h"
#include <string.h>


int oldTicks;
int fakePalette;


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


unsigned char * loadRLE (FILE *f, int size) {

  int rle, pos, byte, count, next;
  char *buffer;

  // Determine the byte that follows the block
  next = fgetc(f);
  next += fgetc(f) << 8;
  next += ftell(f);

  // Allocate memory for the decompressed data
  buffer = malloc(size);

  pos = 0;

  while (pos < size) {

    rle = fgetc(f);

    if (rle > 127) {

      byte = fgetc(f);

      for (count = 0; count < (rle & 127); count++) {

        buffer[pos++] = byte;
        if (pos >= size) break;

      }

    } else if (rle > 0) {

      for (count = 0; count < rle; count++) {

        buffer[pos++] = fgetc(f);
        if (pos >= size) break;

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


SDL_Surface * createSurface (unsigned char * pixels, int width, int height) {

  SDL_Surface *ret;

  // Create the surface
  ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);

  // Set the surface's palette
  SDL_SetPalette(ret, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 256);

  // Upload pixel data to the surface
  if (SDL_MUSTLOCK(ret)) SDL_LockSurface(ret);
  memcpy(ret->pixels, pixels, width * height);
  if (SDL_MUSTLOCK(ret)) SDL_UnlockSurface(ret);

  // Free redundant pixel data
  free(pixels);

  return ret;

}


int loadMain (void) {

  FILE *f;
  unsigned char *pixels;
  int rle, pos, index, count;


  // Generate a placeholder palette

  for (count = 0; count < 256; count++) {

    realPalette[count].r = count;
    realPalette[count].g = count;
    realPalette[count].b = count;

  }

  SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 256);


  f = fopenFromPath("panel.000");

  // Load the panel

  panel = createSurface(loadRLE(f, 320 * 32), 320, 32);


  // Load the large panel font
  // Starts at 4691 and goes 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-:*

  pixels = malloc(320 * 8);
  
  pixels[0] = 31;

  pos = 1;

  fseek(f, 4691, SEEK_SET);

  // RLE decompression and horizontal to vertical character rearrangement
  while (pos < 320 * 8) {

    rle = fgetc(f);

    if (rle >= 128) {

      index = fgetc(f);

      for (count = 0; count < (rle & 127); count++) {

        pixels[(pos & 7) + ((pos / 320) * 8) + (((pos % 320)>>3) * 8 * 8)] =
          index;
        pos++;

      }

    } else if (rle > 0) {

      for (count = 0; count < rle; count++) {

        pixels[(pos & 7) + ((pos / 320) * 8) + (((pos % 320)>>3) * 8 * 8)] =
          fgetc(f);
        pos++;

      }

    } else break;

  }

  // Create ASCII->font map
  for (count = 0; count < 42; count++) panelBigFont.map[count] = 39;
  panelBigFont.map[42] = 38;
  for (count = 43; count < 91; count++) panelBigFont.map[count] = count - 55;
  panelBigFont.map[45] = 36;
  for (count = 46; count < 48; count++) panelBigFont.map[count] = 39;
  for (; count < 58; count++) panelBigFont.map[count] = count - 48;
  panelBigFont.map[58] = 37;
  for (count = 59; count < 65; count++) panelBigFont.map[count] = 39;
  for (; count < 91; count++) panelBigFont.map[count] = count - 55;
  for (; count < 97; count++) panelBigFont.map[count] = 39;
  for (; count < 123; count++) panelBigFont.map[count] = count - 87;
  for (; count < 128; count++) panelBigFont.map[count] = 39;

  // Set font dimensions
  panelBigFont.w = 8;
  panelBigFont.h = 8;

  panelBigFont.pixels = createSurface(pixels, 8, 320);


  // Load the small panel font
  // Starts at 6975 and goes 0123456789oo (where oo = infinity)

  pixels = malloc(320 * 7);
  
  pos = 0;

  fseek(f, 6975, SEEK_SET);

  // RLE decompression and horizontal to vertical character rearrangement
  while (pos < 320 * 7) {

    rle = fgetc(f);

    if (rle >= 128) {

      index = fgetc(f);

      for (count = 0; count < (rle & 127); count++) {

        pixels[(pos & 7) + ((pos / 320) * 8) + (((pos % 320)>>3) * 8 * 7)] =
          index;
        pos++;

      }

    } else if (rle > 0) {

      for (count = 0; count < rle; count++) {

        pixels[(pos & 7) + ((pos / 320) * 8) + (((pos % 320)>>3) * 8 * 7)] =
          fgetc(f);
        pos++;

      }

    } else break;

  }

  fclose(f);

  // Create ASCII->font map
  for (count = 0; count < 48; count++) panelSmallFont.map[count] = 12;
  // Use :; to represent oo
  for (; count < 60; count++) panelSmallFont.map[count] = count - 48;
  for (; count < 128; count++) panelSmallFont.map[count] = 12;

  // Set font dimensions
  panelSmallFont.w = 8;
  panelSmallFont.h = 7;

  panelSmallFont.pixels = createSurface(pixels, 8, 280);

  return 0;

}


void updatePalettes (void) {

  // Make palette changes invisible until the next draw
  SDL_FillRect(screen, NULL, 31);
  SDL_Flip(screen);

  SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 256);
  SDL_SetPalette(panel, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 256);
  SDL_SetPalette(panelBigFont.pixels, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0,
                 256);
  SDL_SetPalette(panelSmallFont.pixels, SDL_LOGPAL | SDL_PHYSPAL, realPalette,
                 0, 256);

  return;

}


int loop (void) {

  SDL_Event event;
  paletteEffect *currentPE;
  SDL_Color shownPalette[256];
  int count, x, y;


  // Show everything that has been drawn so far
  SDL_Flip(screen);


  // Calculate frame rate
  count = SDL_GetTicks();
  fps = 1000.0/((float)(count - oldTicks));
  if (count - oldTicks > 100) oldTicks = count - 100;
  spf = ((float)(count - oldTicks))/1000.0;
  oldTicks = count;


  // Process system events
  while (SDL_PollEvent(&event)) {

    switch (event.type) {

      case SDL_KEYDOWN:

        // If Alt + Enter has been pressed, go to full screen
        if ((event.key.keysym.sym == SDLK_RETURN) &&
            (event.key.keysym.mod & KMOD_ALT)        ) {

          fullscreen = !fullscreen;

          if (fullscreen) {

            SDL_ShowCursor(SDL_DISABLE);
            screen = SDL_SetVideoMode(screenW, screenH, 8,
                                      SDL_FULLSCREEN | SDL_DOUBLEBUF |
                                      SDL_HWSURFACE | SDL_HWPALETTE);
            SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0,
                           256);

            // A real 8-bit display is quite likely if the user has the right
            // video card, the right video drivers, the right version of
            // DirectX/whatever, and the right version of SDL. In other words,
            // it's not likely enough.
            // If a real palette is assumed when
            // a) There really is a real palette, there will be an extremely
            // small speed gain
            // b) The palette is emulated, there will be a HUGE speed loss
            // Therefore, assume the palette is emulated
            // To do: Find a better way!
            fakePalette = 1;

          } else {

            screen = SDL_SetVideoMode(screenW, screenH, 8,
                                      SDL_RESIZABLE | SDL_DOUBLEBUF |
                                      SDL_HWSURFACE | SDL_HWPALETTE  );
            SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0,
                           256);
            SDL_ShowCursor(SDL_ENABLE);

            // Assume that in windowed mode the palette is being emulated
            // This is extremely likely
            // To do: Find a better way!
            fakePalette = 1;

          }

        }

        if (event.key.keysym.sym == SDLK_ESCAPE) return RETURN;

        // The absence of a break statement is intentional

      case SDL_KEYUP:

        for (count = 0; count < KEYS; count++)
          if (event.key.keysym.sym == keys[count].key)
            keys[count].state = event.key.state;

        break;

      case SDL_VIDEORESIZE:

        screenW = event.resize.w;
        screenH = event.resize.h;
        if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
        else bgScale = ((screenH - 34) / 100) + 1;
        screen = SDL_SetVideoMode(screenW, screenH, 8,
                                  SDL_RESIZABLE | SDL_DOUBLEBUF |
                                  SDL_HWSURFACE | SDL_HWPALETTE  );

        // The absence of a break statement is intentional

      case SDL_VIDEOEXPOSE:

        SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL, realPalette, 0, 256);

        break;

      case SDL_QUIT:

        return QUIT;

    }

  }

  // Apply palette effects

  currentPE = firstPE;

  // If the palette is being emulated, compile all palette changes and apply
  // them all at once.
  // If the palette is being used directly, apply all palette effects directly
  if (fakePalette) {

    memcpy(shownPalette, realPalette, sizeof(SDL_Color) * 256);

    while (currentPE != NULL) {

      switch (currentPE->type) {

        case PE_FADE:

          if (currentPE->position) {

            for (count = currentPE->first;
                 count < currentPE->first + currentPE->amount; count++) {

              shownPalette[count].r = realPalette[count].r *
                                      currentPE->position;
              shownPalette[count].g = realPalette[count].g *
                                      currentPE->position;
              shownPalette[count].b = realPalette[count].b *
                                      currentPE->position;

            }

            currentPE->position -= spf / currentPE->speed;

          } else {

            memset(shownPalette + currentPE->first, 0,
                   sizeof(SDL_Color) * currentPE->amount);

          }

          break;

        case PE_1D:

          currentPE->position = currentPE->amount - 1 -
                                ((int)(currentPE->position * currentPE->speed)
                                 % currentPE->amount                          );

          // The lack of a break statement is intentional

        case PE_ROTATE:

          for (count = 0; count < currentPE->amount; count++) {

            memcpy(shownPalette + currentPE->first + count,
                   realPalette + currentPE->first +
                   ((count + (int)(currentPE->position)) % currentPE->amount),
                   sizeof(SDL_Color));

          }

          currentPE->position -= spf * currentPE->speed;
          while (currentPE->position < 0)
            currentPE->position += currentPE->amount;

          break;

        case PE_SKY:

          count = (int)((currentPE->position * currentPE->speed) / bgScale) % 255;

          if (count > 255 - currentPE->amount) {

            memcpy(shownPalette + currentPE->first, bgPalette + count,
                   sizeof(SDL_Color) * (255 - count));
            memcpy(shownPalette + currentPE->first + (255 - count), bgPalette,
                   sizeof(SDL_Color) * (count + currentPE->amount - 255));

          } else {

            memcpy(shownPalette + currentPE->first, bgPalette + count,
                   sizeof(SDL_Color) * currentPE->amount);

          }

          break;

        case PE_2D:

          x = ((LW * TW) - *((short int *)&(currentPE->position))) *
              currentPE->speed;
          y = ((LH * TH) - *((short int *)&(currentPE->position) + 1)) *
              currentPE->speed;

          for (count = 0; count < currentPE->amount >> 3; count++) {

            int j;

            for (j = 0; j < 8; j++) {

              memcpy(shownPalette + currentPE->first + (count << 3) + j,
                     realPalette + currentPE->first +
                     (((count + y) % 8) << 3) + ((j + x) % 8),
                     sizeof(SDL_Color));

            }

          }

          break;

        case PE_WATER:

          // To do: Underwater darkness palette effect
          break;

      }

      currentPE = currentPE->next;

    }

    if (firstPE != NULL)
    SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette, 0, 256);

  } else {

    while (currentPE != NULL) {

      switch (currentPE->type) {

        case PE_FADE:

          if (currentPE->position) {

            for (count = currentPE->first;
                 count < currentPE->first + currentPE->amount; count++) {

              shownPalette[count].r = realPalette[count].r *
                                      currentPE->position;
              shownPalette[count].g = realPalette[count].g *
                                      currentPE->position;
              shownPalette[count].b = realPalette[count].b *
                                      currentPE->position;

            }

            currentPE->position -= spf / currentPE->speed;

          } else {

            memset(shownPalette + currentPE->first, 0,
                   sizeof(SDL_Color) * currentPE->amount);

          }

          SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + currentPE->first,
                         currentPE->first, currentPE->amount);

          break;

        case PE_1D:

          currentPE->position = currentPE->amount - 1 -
                                ((int)(currentPE->position * currentPE->speed)
                                 % currentPE->amount                          );

          // The lack of a break statement is intentional

        case PE_ROTATE:

          for (count = 0; count < currentPE->amount; count++) {

            memcpy(shownPalette + currentPE->first + count,
                   realPalette + currentPE->first +
                   ((count + (int)(currentPE->position)) % currentPE->amount),
                   sizeof(SDL_Color));

          }

          currentPE->position -= spf * currentPE->speed;
          while (currentPE->position < 0)
            currentPE->position += currentPE->amount;

          SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + currentPE->first,
                         currentPE->first, currentPE->amount);

          break;

        case PE_SKY:

          count = (int)((currentPE->position * currentPE->speed) / bgScale) %
                  255;

          if (count > 255 - currentPE->amount) {

            SDL_SetPalette(screen, SDL_PHYSPAL, bgPalette + count,
                           currentPE->first, 255 - count);
            SDL_SetPalette(screen, SDL_PHYSPAL, bgPalette,
                           currentPE->first + (255 - count),
                           count + currentPE->amount - 255);

          } else {

            SDL_SetPalette(screen, SDL_PHYSPAL, bgPalette + count,
                           currentPE->first, currentPE->amount);

          }

          break;

        case PE_2D:

          x = ((LW * TW) - *((short int *)&(currentPE->position))) *
              currentPE->speed;
          y = ((LH * TH) - *((short int *)&(currentPE->position) + 1)) *
              currentPE->speed;

          for (count = 0; count < currentPE->amount >> 3; count++) {

            int j;

            for (j = 0; j < 8; j++) {

              memcpy(shownPalette + currentPE->first + (count << 3) + j,
                     realPalette + currentPE->first +
                     (((count + y) % 8) << 3) + ((j + x) % 8),
                     sizeof(SDL_Color));

            }

          }

          SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + currentPE->first,
                         currentPE->first, currentPE->amount);

          break;

        case PE_WATER:

          // To do: Underwater darkness palette effect
          break;

      }

      currentPE = currentPE->next;

    }

  }


  return CONTINUE;

}


void quit (void) {

  SDL_FreeSurface(panel);
  SDL_FreeSurface(panelBigFont.pixels);
  SDL_FreeSurface(panelSmallFont.pixels);

  free(path);

  SDL_Quit();

  exit(0);

}


int main(int argc, char *argv[]) {

  // Temp: Load level given on command line
  if (argc < 2) {

    int count;

    count = strlen(argv[0]) - 1;

#ifdef WIN32
    while ((argv[0][count] != '\\') && (count >= 0)) count--;
#else
    while ((argv[0][count] != '/') && (count >= 0)) count--;
#endif

    path = malloc(count + 2);
    if (count >= 0) {

      memcpy(path, argv[0], count + 1);
      path[count + 1] = 0;

    } else path[0] = 0;

  } else {

#ifdef WIN32
    if (argv[1][strlen(argv[1]) - 1] != '\\') {
#else
    if (argv[1][strlen(argv[1]) - 1] != '/') {
#endif

      path = malloc(strlen(argv[1]) + 2);
      strcpy(path, argv[1]);

#ifdef WIN32
      strcat(path, "\\");
#else
      strcat(path, "/");
#endif

    } else {

      path = malloc(strlen(argv[1]) + 1);
      strcpy(path, argv[1]);

    }

  }

  // Initialise global variables

  // To do: Load keys from config file
  keys[K_UP].key      = SDLK_UP;
  keys[K_DOWN].key    = SDLK_DOWN;
  keys[K_LEFT].key    = SDLK_LEFT;
  keys[K_RIGHT].key   = SDLK_RIGHT;
#ifdef WIN32
  keys[K_JUMP].key    = SDLK_RALT;
#else
  keys[K_JUMP].key    = SDLK_LALT;
#endif
  keys[K_FIRE].key    = SDLK_SPACE;
  keys[K_CHANGE].key  = SDLK_RCTRL;

  cutscene.pixels = NULL;
  firstPE = NULL;

  // Todo: Different resolutions
  screenW = 320;
  screenH = 200;
  fullscreen = 0;
  bgScale = 2;

  spf = 0.02f;
  fps = 50.0f;   // Arbitrarily chosen starting speed

  // Initialise SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {

    fprintf(stderr, "Could not start SDL: %s\n", SDL_GetError());

    return -1;

  }

  // Create the game's window
  if ((screen = SDL_SetVideoMode(screenW, screenH, 8,
                                 SDL_RESIZABLE | SDL_DOUBLEBUF |
                                 SDL_HWSURFACE | SDL_HWPALETTE  )) == NULL) {

    fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());

    return -1;

  }

  // Assume that in windowed mode the palette is being emulated
  // This is extremely likely
  // To do: Find a better way!
  fakePalette = 1;

  SDL_WM_SetCaption("OpenJazz", NULL);


  // Load universal game data

  if (loadMain() == -1) {

    SDL_Quit();
    return -1;

  }


  // Temp: Load the first level

  nextworld = 0;
  nextlevel = 0;
  if (loadMenu() == -1) quit();

  // Establish timing
  oldTicks = SDL_GetTicks() - 20;

  menuLoop();

  quit();

  return 0;

}


