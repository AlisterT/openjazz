
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
 * Contains the main function.
 *
 */


#define Extern
#include "OpenJazz.h"
#include <string.h>


// Functions in font.c
extern font * loadFont   (char * fn);
extern void   freeFont   (font * f);

// Functions in sound.c
extern int  loadSounds (char * fn);
extern void freeSounds (void);


// Palettes in menu.c
extern SDL_Color menuPalettes[4][256];


int oldTicks;
int fakePalette;


int loadMain (void) {

  FILE *f;
  unsigned char *pixels, *sorted;
  int rle, pos, index, count, x, y;


  // Initialise video settings
  screenW = 320;
  screenH = 200;
#ifndef FULLSCREEN_ONLY
  fullscreen = 0;
#endif
  bgScale = 2;
  // Assume that in windowed mode the palette is being emulated
  // This is extremely likely
  // To do: Find a better way!
  fakePalette = 1;


  // Initialise controls
  keys[C_UP].key     = SDLK_UP;
  keys[C_DOWN].key   = SDLK_DOWN;
  keys[C_LEFT].key   = SDLK_LEFT;
  keys[C_RIGHT].key  = SDLK_RIGHT;
  #ifdef WIN32
  keys[C_JUMP].key   = SDLK_RALT;
  keys[C_FIRE].key   = SDLK_SPACE;
  #else
  keys[C_JUMP].key   = SDLK_SPACE;
  keys[C_FIRE].key   = SDLK_LALT;
  #endif
  keys[C_CHANGE].key = SDLK_RCTRL;
  keys[C_ENTER].key  = SDLK_RETURN;
  keys[C_ESCAPE].key = SDLK_ESCAPE;
  keys[C_STATS].key  = SDLK_F9;
  keys[C_PAUSE].key  = SDLK_p;

  buttons[C_UP].button = -1;
  buttons[C_DOWN].button = -1;
  buttons[C_LEFT].button = -1;
  buttons[C_RIGHT].button = -1;
  buttons[C_JUMP].button = 1;
  buttons[C_FIRE].button = 0;
  buttons[C_CHANGE].button = 3;
  buttons[C_ENTER].button = 0;
  buttons[C_ESCAPE].button = -1;
  buttons[C_STATS].button = -1;
  buttons[C_PAUSE].button = -1;

  axes[C_UP].axis = 1;
  axes[C_UP].direction = 0;
  axes[C_DOWN].axis = 1;
  axes[C_DOWN].direction = 1;
  axes[C_LEFT].axis = 0;
  axes[C_LEFT].direction = 0;
  axes[C_RIGHT].axis = 0;
  axes[C_RIGHT].direction = 1;
  axes[C_JUMP].axis = -1;
  axes[C_FIRE].axis = -1;
  axes[C_CHANGE].axis = -1;
  axes[C_ENTER].axis = -1;
  axes[C_ESCAPE].axis = -1;
  axes[C_STATS].axis = -1;
  axes[C_PAUSE].axis = -1;

  for (count = 0; count < CONTROLS; count++) {

    controls[count].time = 0;
    controls[count].state = SDL_RELEASED;

  }


  // Create the player's name
  localPlayerName = malloc(sizeof(char) * (STRING_LENGTH + 1));
  strcpy(localPlayerName, "JAZZ");


  // Open config file
  f = fopen("openjazz.cfg", "rb");

  // Check that the config file was opened, and has the correct version
  if ((f != NULL) && (fgetc(f) == 1)) {

    // Read video settings
    screenW = fgetc(f);
    screenW += fgetc(f) << 8;
    screenH = fgetc(f);
    screenH += fgetc(f) << 8;
#ifdef FULLSCREEN_ONLY
    fgetc(f);
#else
    fullscreen = fgetc(f);
#endif
    if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
    else bgScale = ((screenH - 34) / 100) + 1;

    // Read controls
    for (count = 0; count < CONTROLS - 4; count++)
      keys[count].key = loadInt(f);

    for (count = 0; count < CONTROLS; count++)
      buttons[count].button = loadInt(f);

    for (count = 0; count < CONTROLS; count++) {
      axes[count].axis = loadInt(f);
      axes[count].direction = loadInt(f);
    }

    // Read the player's name
    for (count = 0; count < STRING_LENGTH; count++)
      localPlayerName[count] = fgetc(f);

    localPlayerName[STRING_LENGTH] = 0;

    fclose(f);

  } else {

    printf("Valid configuration file not found.\n");

  }

  // Create the game's window

#ifdef FULLSCREEN_ONLY
  if ((screen = SDL_SetVideoMode(screenW, screenH, 8,
                                 SDL_FULLSCREEN | SDL_DOUBLEBUF |
                                 SDL_HWSURFACE | SDL_HWPALETTE   )) == NULL) {
#else
  if ((screen = SDL_SetVideoMode(screenW, screenH, 8,
                                 SDL_RESIZABLE | SDL_DOUBLEBUF |
                                 SDL_HWSURFACE | SDL_HWPALETTE  )) == NULL) {
#endif
    fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());

    return -1;

  }

  SDL_WM_SetCaption("OpenJazz", NULL);


  if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);


  // Generate the logical palette
  for (count = 0; count < 256; count++) {

    logicalPalette[count].r = count;
    logicalPalette[count].g = count;
    logicalPalette[count].b = count;

  }

  SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);


  // Default level & difficulty settings

  currentLevel = malloc(11);
  strcpy(currentLevel, "level0.000");

  nextLevel = malloc(11);
  strcpy(nextLevel, "level0.000");

  difficulty = 1;


  // Load the panel

  f = fopenFromPath("panel.000");

  if (f == NULL) {

    free(currentLevel);
    free(nextLevel);

    return FAILURE;

  }

  // Load the panel background
  panel = loadSurface(f, 320, 32);


  // Load the large panel font
  // Starts at 4691 and goes 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-:.

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
  for (count = 0; count < 45; count++) panelBigFont.map[count] = 39;
  panelBigFont.map[45] = 36;
  panelBigFont.map[46] = 38;
  for (count = 47; count < 48; count++) panelBigFont.map[count] = 39;
  for (; count < 58; count++) panelBigFont.map[count] = count - 48;
  panelBigFont.map[58] = 37;
  for (count = 59; count < 65; count++) panelBigFont.map[count] = 39;
  for (; count < 91; count++) panelBigFont.map[count] = count - 55;
  for (; count < 97; count++) panelBigFont.map[count] = 39;
  for (; count < 123; count++) panelBigFont.map[count] = count - 87;
  for (; count < 128; count++) panelBigFont.map[count] = 39;

  // Set font dimensions
  panelBigFont.w = malloc(40);
  for (count = 0; count < 40; count++) panelBigFont.w[count] = 8;
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

  // Create ASCII->font map
  for (count = 0; count < 48; count++) panelSmallFont.map[count] = 12;
  // Use :; to represent the infinity symbol
  for (; count < 60; count++) panelSmallFont.map[count] = count - 48;
  for (; count < 128; count++) panelSmallFont.map[count] = 12;

  // Set font dimensions
  panelSmallFont.w = malloc(13);
  for (count = 0; count < 13; count++) panelSmallFont.w[count] = 8;
  panelSmallFont.h = 7;

  panelSmallFont.pixels = createSurface(pixels, 8, 280);

  // Load the panel's ammo graphics

  fseek(f, 7537, SEEK_SET);
  sorted = malloc(64 * 27);
  pixels = loadRLE(f, 64 * 27);

  for (y = 0; y < 27; y++) {

    for (x = 0; x < 64; x++)
      sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

  }

  panelAmmo[0] = createSurface(sorted, 64, 27);
  sorted = pixels; // Re-use the allocated memory

  fseek(f, 8264, SEEK_SET);
  pixels = loadRLE(f, 64 * 27);

  for (y = 0; y < 27; y++) {

    for (x = 0; x < 64; x++)
      sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

  }

  panelAmmo[1] = createSurface(sorted, 64, 27);
  sorted = pixels; // Re-use the allocated memory

  fseek(f, 9550, SEEK_SET);
  pixels = loadRLE(f, 64 * 27);

  for (y = 0; y < 27; y++) {

    for (x = 0; x < 64; x++)
      sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

  }

  panelAmmo[2] = createSurface(sorted, 64, 27);
  sorted = pixels; // Re-use the allocated memory

  fseek(f, 11060, SEEK_SET);
  pixels = loadRLE(f, 64 * 27);

  for (y = 0; y < 27; y++) {

    for (x = 0; x < 64; x++)
      sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

  }

  panelAmmo[3] = createSurface(sorted, 64, 27);
  sorted = pixels; // Re-use the allocated memory

  fseek(f, 12258, SEEK_SET);
  pixels = loadRLE(f, 64 * 27);

  for (y = 0; y < 27; y++) {

    for (x = 0; x < 64; x++)
      sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

  }

  panelAmmo[4] = createSurface(sorted, 64, 27);
  free(pixels);

  fclose(f);

  // Load real fonts
  font2 = loadFont("font2.0fn");
  fontbig = loadFont("fontbig.0fn");
  fontiny = loadFont("fontiny.0fn");
  fontmn1 = loadFont("fontmn1.0fn");
  fontmn2 = loadFont("fontmn2.0fn");

  firstPE = NULL;


  // Establish arbitrary timing
  mspf = 20;
  fps = 50.0f;
  oldTicks = SDL_GetTicks() - 20;


  return SUCCESS;

}


void releaseControl (int control) {

  controls[control].time = oldTicks + 500;
  controls[control].state = SDL_RELEASED;

  return;

}


void update (void) {

  int ticks;

  // Show everything that has been drawn so far
  SDL_Flip(screen);

  // Calculate frame rate and key timing
  ticks = SDL_GetTicks();
  mspf = ticks - oldTicks;
  fps = 1000.0f / ((float)(mspf));
  if (mspf > 100) mspf = 100;
  oldTicks = ticks;

  return;

}


int loop (void) {

  SDL_Event event;
  paletteEffect *currentPE;
  SDL_Color shownPalette[256];
  int count, x, y;


  update();


  // Process system events
  while (SDL_PollEvent(&event)) {

    switch (event.type) {

      case SDL_KEYDOWN:

#ifndef FULLSCREEN_ONLY
        // If Alt + Enter has been pressed, go to full screen
        if ((event.key.keysym.sym == SDLK_RETURN) &&
            (event.key.keysym.mod & KMOD_ALT)        ) {

          fullscreen = !fullscreen;

          if (fullscreen) {

            SDL_ShowCursor(SDL_DISABLE);
            screen = SDL_SetVideoMode(screenW, screenH, 8,
                                      SDL_FULLSCREEN | SDL_DOUBLEBUF |
                                      SDL_HWSURFACE | SDL_HWPALETTE);
            SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
            SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

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
            SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
            SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);
            SDL_ShowCursor(SDL_ENABLE);

            // Assume that in windowed mode the palette is being emulated
            // This is extremely likely
            // To do: Find a better way!
            fakePalette = 1;

          }

        }

        // The absence of a break statement is intentional
#endif

      case SDL_KEYUP:

        for (count = 0; count < CONTROLS; count++)
          if (event.key.keysym.sym == keys[count].key)
            keys[count].state = event.key.state;

        break;

      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:

        for (count = 0; count < CONTROLS; count++)
          if (event.jbutton.button == buttons[count].button)
            buttons[count].state = event.jbutton.state;

        break;

      case SDL_JOYAXISMOTION:

        for (count = 0; count < CONTROLS; count++)
          if (event.jaxis.axis == axes[count].axis) {

            if (!axes[count].direction && (event.jaxis.value < -16384))
              axes[count].state = SDL_PRESSED;

            else if (axes[count].direction && (event.jaxis.value > 16384))
              axes[count].state = SDL_PRESSED;

            else axes[count].state = SDL_RELEASED;

          }

      break;

#ifndef FULLSCREEN_ONLY
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

        SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
        SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

        break;
#endif

      case SDL_QUIT:

        return QUIT;

    }

  }

  // Apply controls to universal control tracking
  for (count = 0; count < CONTROLS; count++) {

    if ((keys[count].state == SDL_PRESSED) ||
        (buttons[count].state == SDL_PRESSED) || 
        (axes[count].state == SDL_PRESSED)      ) {

      if (controls[count].time < oldTicks) controls[count].state = SDL_PRESSED;

    } else {

      controls[count].time = 0;
      controls[count].state = SDL_RELEASED;

    }

  }


  // Apply palette effects

  currentPE = firstPE;

  // If the palette is being emulated, compile all palette changes and apply
  // them all at once.
  // If the palette is being used directly, apply all palette effects directly
  // Don't apply effects to the second menu palette
  if (fakePalette && (currentPalette != menuPalettes[1])) {

    memcpy(shownPalette, currentPalette, sizeof(SDL_Color) << 8);

    while (currentPE) {

      switch (currentPE->type) {

        case PE_FADE:

          if (currentPE->position) {

            for (count = currentPE->first;
                 count < currentPE->first + currentPE->amount; count++) {

              shownPalette[count].r = (currentPalette[count].r *
                                       currentPE->position) >> 10;
              shownPalette[count].g = (currentPalette[count].g *
                                       currentPE->position) >> 10;
              shownPalette[count].b = (currentPalette[count].b *
                                       currentPE->position) >> 10;

            }

            currentPE->position -= (mspf << 10) / currentPE->speed;

          } else {

            memset(shownPalette + currentPE->first, 0,
                   sizeof(SDL_Color) * currentPE->amount);

          }

          break;

        case PE_1D:

          currentPE->position = currentPE->amount - 1 -
                                (((currentPE->position * currentPE->speed) >>
                                  10)
                                 % currentPE->amount                          );

          // The lack of a break statement is intentional

        case PE_ROTATE:

          for (count = 0; count < currentPE->amount; count++) {

            memcpy(shownPalette + currentPE->first + count,
                   currentPalette + currentPE->first +
                   ((count + (currentPE->position >> 10)) % currentPE->amount),
                   sizeof(SDL_Color));

          }

          currentPE->position -= (mspf * currentPE->speed) >> 10;
          while (currentPE->position < 0)
            currentPE->position += currentPE->amount << 10;

          break;

        case PE_SKY:

          count = (((currentPE->position * currentPE->speed) / bgScale) >> 20) %
                  255;

          if (count > 255 - currentPE->amount) {

            memcpy(shownPalette + currentPE->first, skyPalette + count,
                   sizeof(SDL_Color) * (255 - count));
            memcpy(shownPalette + currentPE->first + (255 - count), skyPalette,
                   sizeof(SDL_Color) * (count + currentPE->amount - 255));

          } else {

            memcpy(shownPalette + currentPE->first, skyPalette + count,
                   sizeof(SDL_Color) * currentPE->amount);

          }

          break;

        case PE_2D:

          x = (((256 * 32) - *((short int *)&(currentPE->position))) *
               currentPE->speed) >> 10;
          y = (((64 * 32) - *((short int *)&(currentPE->position) + 1)) *
               currentPE->speed) >> 10;

          for (count = 0; count < currentPE->amount >> 3; count++) {

            int j;

            for (j = 0; j < 8; j++) {

              memcpy(shownPalette + currentPE->first + (count << 3) + j,
                     currentPalette + currentPE->first +
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

    if (firstPE) SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette, 0, 256);

  } else if (currentPalette != menuPalettes[1]) {

    while (currentPE) {

      switch (currentPE->type) {

        case PE_FADE:

          if (currentPE->position) {

            for (count = currentPE->first;
                 count < currentPE->first + currentPE->amount; count++) {

              shownPalette[count].r = (currentPalette[count].r *
                                       currentPE->position) >> 10;
              shownPalette[count].g = (currentPalette[count].g *
                                       currentPE->position) >> 10;
              shownPalette[count].b = (currentPalette[count].b *
                                       currentPE->position) >> 10;

            }

            currentPE->position -= (mspf << 10) / currentPE->speed;

          } else {

            memset(shownPalette + currentPE->first, 0,
                   sizeof(SDL_Color) * currentPE->amount);

          }

          SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + currentPE->first,
                         currentPE->first, currentPE->amount);

          break;

        case PE_1D:

          currentPE->position = currentPE->amount - 1 -
                                (((currentPE->position * currentPE->speed) >>
                                  10)
                                 % currentPE->amount                          );

          // The lack of a break statement is intentional

        case PE_ROTATE:

          for (count = 0; count < currentPE->amount; count++) {

            memcpy(shownPalette + currentPE->first + count,
                   currentPalette + currentPE->first +
                   ((count + (currentPE->position >> 10)) % currentPE->amount),
                   sizeof(SDL_Color));

          }

          currentPE->position -= (mspf * currentPE->speed) >> 10;
          while (currentPE->position < 0)
            currentPE->position += currentPE->amount << 10;

          SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + currentPE->first,
                         currentPE->first, currentPE->amount);

          break;

        case PE_SKY:

          count = (((currentPE->position * currentPE->speed) / bgScale) >> 20) %
                  255;

          if (count > 255 - currentPE->amount) {

            SDL_SetPalette(screen, SDL_PHYSPAL, skyPalette + count,
                           currentPE->first, 255 - count);
            SDL_SetPalette(screen, SDL_PHYSPAL, skyPalette,
                           currentPE->first + (255 - count),
                           count + currentPE->amount - 255);

          } else {

            SDL_SetPalette(screen, SDL_PHYSPAL, skyPalette + count,
                           currentPE->first, currentPE->amount);

          }

          break;

        case PE_2D:

          x = (((256 * 32) - *((short int *)&(currentPE->position))) *
               currentPE->speed) >> 10;
          y = (((64 * 32) - *((short int *)&(currentPE->position) + 1)) *
               currentPE->speed) >> 10;

          for (count = 0; count < currentPE->amount >> 3; count++) {

            int j;

            for (j = 0; j < 8; j++) {

              memcpy(shownPalette + currentPE->first + (count << 3) + j,
                     currentPalette + currentPE->first +
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


  return SUCCESS;

}


void freeMain (void) {

  FILE *f;
  int count;

  SDL_FreeSurface(panel);

  SDL_FreeSurface(panelBigFont.pixels);
  free(panelBigFont.w);

  SDL_FreeSurface(panelSmallFont.pixels);
  free(panelSmallFont.w);

  freeFont(font2);
  freeFont(fontbig);
  freeFont(fontiny);
  freeFont(fontmn1);
  freeFont(fontmn2);

  SDL_FreeSurface(panelAmmo[0]);
  SDL_FreeSurface(panelAmmo[1]);
  SDL_FreeSurface(panelAmmo[2]);
  SDL_FreeSurface(panelAmmo[3]);
  SDL_FreeSurface(panelAmmo[4]);

  free(currentLevel);
  free(nextLevel);

  // Open config file
  f = fopen("openjazz.cfg", "wb");

  // Check that the config file was opened
  if (f != NULL) {

    // Write the version number
    fputc(1, f);

    // Write video settings
    fputc(screenW & 255, f);
    fputc(screenW >> 8, f);
    fputc(screenH & 255, f);
    fputc(screenH >> 8, f);
#ifdef FULLSCREEN_ONLY
    fputc(1, f);
#else
    fputc(fullscreen, f);
#endif

    // Write controls
    for (count = 0; count < CONTROLS - 4; count++)
      storeInt(keys[count].key, f);

    for (count = 0; count < CONTROLS; count++)
      storeInt(buttons[count].button, f);

    for (count = 0; count < CONTROLS; count++) {
      storeInt(axes[count].axis, f);
      storeInt(axes[count].direction, f);
    }

    // Write the player's name
    for (count = 0; count < STRING_LENGTH; count++)
      fputc(localPlayerName[count], f);

    fclose(f);

  } else {

    printf("Could not write configuration file.\n");

  }

  free(localPlayerName);
  free(path);

  return;

}


int main(int argc, char *argv[]) {

  // Find path

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


  // Initialise SDL

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK |
              SDL_INIT_TIMER                                       ) < 0) {

    fprintf(stderr, "Could not start SDL: %s\n", SDL_GetError());

    return -1;

  }

  // Load universal game data and establish a window

  if (loadMain()) {

    SDL_Quit();

    return -1;

  }


  // Show the startup cutscene
//  if (runScene("startup.0sc") != QUIT) {

    // Run the menu
    if (runMenu() != QUIT) {

      // Show the ending cutscene
//      runScene("end.0sc");

    }

//  }


  freeMain();

  SDL_Quit();


  return 0;

}


