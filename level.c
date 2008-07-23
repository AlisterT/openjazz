
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
 * Deals with the running and freeing of ordinary levels.
 *
 */


#define Lextern
#include "level.h"
#include <string.h>


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

  free(pathX);
  free(pathY);

  SDL_FreeSurface(tileSet);

  for (count = 0; count <= sprites; count++)
    SDL_FreeSurface(spriteSet[count].pixels);

  free(spriteSet);

  freeMusic();

  restorePalette(fontmn1->pixels);

  return;

}



int checkMask (fixed x, fixed y) {

  // Anything off the edge of the map is solid
  if ((x < 0) || (y < 0) || (x > (LW * TW << 10)) || (y > (LH * TH << 10)))
    return 1;

  // Event 122 is one-way
  if (grid[y >> 15][x >> 15].event == 122) return 0;

  // Check the mask in the tile in question
  return mask[grid[y >> 15][x >> 15].tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}


int checkMaskDown (fixed x, fixed y) {

  // Anything off the edge of the map is solid
  if ((x < 0) || (y < 0) || (x > (LW * TW << 10)) || (y > (LH * TH << 10)))
    return 1;

  // Check the mask in the tile in question
  return mask[grid[y >> 15][x >> 15].tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}



int playLevelFrame (int ticks) {

  bullet *bul, *prevBul;
  event *evt, *nextEvt;
  fixed dx, dy;
  int x, y;

  // Apply controls to local player
  for (x = 0; x < PCONTROLS; x++)
    localPlayer->controls[x] = controls[x].state;


  // Physics


  // Determine the player's trajectory

  if (localPlayer->controls[C_LEFT] == SDL_PRESSED) {

    if (localPlayer->dx > 0) localPlayer->dx -= 600 * mspf;
    if (localPlayer->dx > -PS_WALK) localPlayer->dx -= 300 * mspf;
    if (localPlayer->dx > -PS_RUN) localPlayer->dx -= 150 * mspf;
    if (localPlayer->dx < -PS_RUN) localPlayer->dx = -PS_RUN;

    localPlayer->facing = 0;

  } else if (localPlayer->controls[C_RIGHT] == SDL_PRESSED) {
    
    if (localPlayer->dx < 0) localPlayer->dx += 600 * mspf;
    if (localPlayer->dx < PS_WALK) localPlayer->dx += 300 * mspf;
    if (localPlayer->dx < PS_RUN) localPlayer->dx += 150 * mspf;
    if (localPlayer->dx > PS_RUN) localPlayer->dx = PS_RUN;

    localPlayer->facing = 1;

  } else {

    if (localPlayer->dx > 0) {

      if (localPlayer->dx < 1000 * mspf) localPlayer->dx = 0;
      else localPlayer->dx -= 1000 * mspf;

    }

    if (localPlayer->dx < 0) {

      if (localPlayer->dx > -1000 * mspf) localPlayer->dx = 0;
      else localPlayer->dx += 1000 * mspf;

    }

  }


  if (localPlayer->floating) {
    
    if (localPlayer->controls[C_UP] == SDL_PRESSED) {

      if (localPlayer->dy > 0) localPlayer->dy -= 600 * mspf;
      if (localPlayer->dy > -PS_WALK) localPlayer->dy -= 300 * mspf;
      if (localPlayer->dy > -PS_RUN) localPlayer->dy -= 150 * mspf;
      if (localPlayer->dy < -PS_RUN) localPlayer->dy = -PS_RUN;

    } else if (localPlayer->controls[C_DOWN] == SDL_PRESSED) {
    
      if (localPlayer->dy < 0) localPlayer->dy += 600 * mspf;
      if (localPlayer->dy < PS_WALK) localPlayer->dy += 300 * mspf;
      if (localPlayer->dy < PS_RUN) localPlayer->dy += 150 * mspf;
      if (localPlayer->dy > PS_RUN) localPlayer->dy = PS_RUN;

    } else {

      if (localPlayer->dy > 0) {

        if (localPlayer->dy < 1000 * mspf) localPlayer->dy = 0;
        else localPlayer->dy -= 1000 * mspf;

      }

      if (localPlayer->dy < 0) {

        if (localPlayer->dy > -1000 * mspf) localPlayer->dy = 0;
        else localPlayer->dy += 1000 * mspf;

      }

    }

    if (eventSet[localPlayer->event][E_MODIFIER] == 29)
      localPlayer->dy = eventSet[localPlayer->event][E_MULTIPURPOSE] * -F20;

    else if (eventSet[localPlayer->event][E_BEHAVIOUR] == 25)
      localPlayer->dy = PS_JUMP;

  } else {

    if (localPlayer->y > localPlayer->jumpY)
      localPlayer->dy = (localPlayer->jumpY - ((2 * F32) + localPlayer->y)) * 4;

    if ((eventSet[localPlayer->event][E_MODIFIER] == 6) ||
        (eventSet[localPlayer->event][E_BEHAVIOUR] == 28) ||
        checkMaskDown(localPlayer->x + F12, localPlayer->y + F8) ||
        checkMaskDown(localPlayer->x + F16, localPlayer->y + F8) ||
        checkMaskDown(localPlayer->x + F20, localPlayer->y + F8)   ) {

      if ((localPlayer->controls[C_JUMP] == SDL_PRESSED) &&
          !checkMask(localPlayer->x + F16, localPlayer->y - F36)) {

        localPlayer->jumpY = localPlayer->y - (localPlayer->jumpHeight +
                                               ((4928 / (fps + 136)) * F1));

        if (localPlayer->dx < 0) {

          localPlayer->jumpY += localPlayer->dx / 8000;

        } else if (localPlayer->dx > 0) {

          localPlayer->jumpY -= localPlayer->dx / 8000;

        }

        localPlayer->event = 0;

      }

      if (localPlayer->controls[C_UP] == SDL_PRESSED) {

        if (localPlayer->lookTime == 0) localPlayer->lookTime = -ticks;

      }

      if (localPlayer->controls[C_DOWN] == SDL_PRESSED) {

        if (localPlayer->lookTime == 0) localPlayer->lookTime = ticks;

      }

    } else {

      localPlayer->lookTime = 0;

    }

    if ((localPlayer->controls[C_JUMP] == SDL_RELEASED) &&
        (eventSet[localPlayer->event][E_MODIFIER] != 29) &&
        (eventSet[localPlayer->event][E_BEHAVIOUR] != 25)  )
      localPlayer->jumpY = 65 * F32;

    if ((localPlayer->controls[C_UP] == SDL_RELEASED) &&
        (localPlayer->controls[C_DOWN] == SDL_RELEASED) )
      localPlayer->lookTime = 0;

    localPlayer->dy += 2750 * mspf;
    if (localPlayer->dy > PS_FALL) localPlayer->dy = PS_FALL;

  }

  if (checkMask(localPlayer->x + F16, localPlayer->y - F24) &&
      (localPlayer->jumpY < localPlayer->y) &&
      (eventSet[localPlayer->event][E_BEHAVIOUR] != 25)       ) {

    localPlayer->jumpY = 65 * F32;

    if ((eventSet[localPlayer->event][E_MODIFIER] != 6) &&
        (eventSet[localPlayer->event][E_BEHAVIOUR] != 28))
      localPlayer->event = 0;

  }
    
  if (localPlayer->y <= localPlayer->jumpY) {

    localPlayer->jumpY = 65 * F32;

    if ((eventSet[localPlayer->event][E_MODIFIER] != 6) &&
        (eventSet[localPlayer->event][E_BEHAVIOUR] != 28))
      localPlayer->event = 0;

  }


  // Apply as much of that trajectory as possible, without going into the
  // scenery

  dx = (localPlayer->dx * mspf) >> 10;
  dy = (localPlayer->dy * mspf) >> 10;

  if (dx >= 0) x = dx >> 12;
  else x = -((-dx) >> 12);

  if (dy >= 0) y = dy >> 12;
  else y = -((-dy) >> 12);

  // First for the vertical component of the trajectory

  while (y > 0) {

    if (checkMaskDown(localPlayer->x + F12, localPlayer->y + F4) ||
        checkMaskDown(localPlayer->x + F16, localPlayer->y + F4) ||
        checkMaskDown(localPlayer->x + F20, localPlayer->y + F4)   ) break;

    localPlayer->y += F4;
    y--;

  }


  while (y < 0) {

    if (checkMask(localPlayer->x + F16, localPlayer->y - F24)) break;

    localPlayer->y -= F4;
    y++;

  }

  if (dy >= 0) dy &= 4095;
  else dy = -((-dy) & 4095);

  if (((dy > 0) &&
      !(checkMaskDown(localPlayer->x + F12, localPlayer->y + dy) ||
        checkMaskDown(localPlayer->x + F16, localPlayer->y + dy) ||
        checkMaskDown(localPlayer->x + F20, localPlayer->y + dy)   )) ||
      ((dy < 0) &&
       !checkMask(localPlayer->x + F16, localPlayer->y + dy - F20)       ))
    localPlayer->y += dy;

  // Then for the horizontal component of the trajectory

  while (x < 0) {

    if (checkMask(localPlayer->x + F2, localPlayer->y - F10)) break;

    localPlayer->x -= F4;
    x++;

  }

  while (x > 0) {

    if (checkMask(localPlayer->x + F30, localPlayer->y - F10)) break;

    localPlayer->x += F4;
    x--;

  }

  if (dx >= 0) dx &= 4095;
  else dx = -((-dx) & 4095);

  if (((dx < 0) &&
       !checkMask(localPlayer->x + F6 + dx, localPlayer->y - F10)) ||
      ((dx > 0) &&
       !checkMask(localPlayer->x + F26 + dx, localPlayer->y - F10))  )
    localPlayer->x += dx;

  // If on an uphill slope, push the player upwards
  if (dx < 0)
    while (checkMask(localPlayer->x + F12, localPlayer->y) &&
           !checkMask(localPlayer->x + F12, localPlayer->y - F20))
      localPlayer->y -= F1;
  else
    while (checkMask(localPlayer->x + F20, localPlayer->y) &&
           !checkMask(localPlayer->x + F20, localPlayer->y - F20))
      localPlayer->y -= F1;



  // Calculate viewport

  // Can we can see below the panel?
  if (localPlayer->viewW > panel->w) localPlayer->viewH = screenH;
  else localPlayer->viewH = screenH - 33;
  localPlayer->viewW = screenW;

  // Record old viewport position
  dx = localPlayer->viewX;
  dy = localPlayer->viewY;

  if (localPlayer->reaction != PR_WON) {

    localPlayer->viewX = localPlayer->x + F8 - (localPlayer->viewW << 9);

    if ((localPlayer->lookTime == 0) ||
        (ticks < 1000 + localPlayer->lookTime) ||
        (ticks < 1000 - localPlayer->lookTime)   ) {

      localPlayer->viewY = localPlayer->y - F24 - (localPlayer->viewH << 9);

    } else if (localPlayer->lookTime > 0) {

      if (ticks < 2000 + localPlayer->lookTime)
        localPlayer->viewY = localPlayer->y -
                             (F24 - (64 *
                                     (ticks -
                                      (1000 + localPlayer->lookTime)))) -
                             (localPlayer->viewH << 9);
      else
        localPlayer->viewY = localPlayer->y - (F24 - F64) - (localPlayer->viewH << 9);

    } else {

      if (ticks < 2000 - localPlayer->lookTime)
        localPlayer->viewY = localPlayer->y -
                             (F24 + (64 *
                                     (ticks -
                                      (1000 - localPlayer->lookTime)))) -
                             (localPlayer->viewH << 9);
      else
        localPlayer->viewY = localPlayer->y - (F24 + F64) -
                             (localPlayer->viewH << 9);

    }

  } else {

    if (checkX << 15 >
          localPlayer->viewX + (localPlayer->viewW << 9) + (160 * mspf))
      localPlayer->viewX += 160 * mspf;
    else if (checkX << 15 <
               localPlayer->viewX + (localPlayer->viewW << 9) - (160 * mspf))
      localPlayer->viewX -= 160 * mspf;

    if (checkY << 15 >
          localPlayer->viewY + (localPlayer->viewH << 9) + (160 * mspf))
      localPlayer->viewY += 160 * mspf;
    else if (checkY << 15 <
               localPlayer->viewY + (localPlayer->viewH << 9) - (160 * mspf))
      localPlayer->viewY -= 160 * mspf;

  }

  // Apply lag proportional to player "speed"
  x = (abs(localPlayer->dx) + abs(localPlayer->dy)) >> 15;

  if (mspf < x) {

    localPlayer->viewX = ((dx * (x - mspf)) + (localPlayer->viewX * mspf)) / x;
    localPlayer->viewY = ((dy * (x - mspf)) + (localPlayer->viewY * mspf)) / x;

  }


  // Ensure viewport is within the level
  if (localPlayer->viewX < 0) localPlayer->viewX = 0;

  if ((localPlayer->viewX >> 10) + localPlayer->viewW >= LW * TW)
    localPlayer->viewX = ((LW * TW) - localPlayer->viewW) << 10;

  if (localPlayer->viewY < 0) localPlayer->viewY = 0;

  if ((localPlayer->viewY >> 10) + localPlayer->viewH >= LH * TH)
    localPlayer->viewY = ((LH * TH) - localPlayer->viewH) << 10;


  // Search for active events
  for (y = (localPlayer->viewY >> 15) - 5;
       y < (((localPlayer->viewY >> 10) + localPlayer->viewH) >> 5) + 5;
       y++                                                              ) {

    for (x = (localPlayer->viewX >> 15) - 5;
         x < (((localPlayer->viewX >> 10) + localPlayer->viewW) >> 5) + 5;
         x++                                                              ) {

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

  if (checkMask(localPlayer->x + F16, localPlayer->y - F24) &&
      (eventSet[localPlayer->event][E_BEHAVIOUR] == 25)       ) {

    localPlayer->jumpY = 65 * F32;
    localPlayer->event = 0;

  }

  // Process active events
  if (pathLength > 0) pathNode = (ticks / 40) % pathLength;

  evt = firstEvent;

  while (evt) {

    if ((evt->anim != E_FINISHANIM) &&
        (grid[evt->gridY][evt->gridX].event != 121) &&
        ((evt->x < localPlayer->viewX - F160) ||
         (evt->x > localPlayer->viewX +
                   (localPlayer->viewW << 10) + F160) ||
         (evt->y < localPlayer->viewY - F160) ||
         (evt->y > localPlayer->viewY +
                   (localPlayer->viewH << 10) + F160)   ) &&
        ((evt->gridX < (localPlayer->viewX >> 15) - 1) ||
         (evt->gridX >
          (((localPlayer->viewX >> 10) + localPlayer->viewW) >> 5) + 1) ||
         (evt->gridY < (localPlayer->viewY >> 15) - 1) ||
         (evt->gridY >
          (((localPlayer->viewY >> 10) + localPlayer->viewH) >> 5) + 1)   )) {

      // If the event and its origin are off-screen, and the event is not
      // in the process of self-destruction, remove it
      nextEvt = evt->next;
      removeEvent(evt);
      evt = nextEvt;

    } else {

      nextEvt = evt->next;
      playEventFrame(evt, ticks);
      evt = nextEvt;

    }

  }


  // Handle spikes
  if ((localPlayer->reaction == PR_NONE) &&

      /* Above */
      (((grid[(localPlayer->y - F20) >> 15]
             [(localPlayer->x + F16) >> 15].event == 126) &&
        checkMask(localPlayer->x + F16, localPlayer->y - F24)  ) ||

       /* Below */
       ((grid[localPlayer->y >> 15]
             [(localPlayer->x + F16) >> 15].event == 126) &&
        checkMaskDown(localPlayer->x + F16, localPlayer->y + F4)  ) ||

       /* To left*/
       ((grid[(localPlayer->y - F10) >> 15]
             [(localPlayer->x + F6) >> 15].event == 126) &&
        checkMask(localPlayer->x + F2, localPlayer->y - F10)  ) ||

       /* To right*/
       ((grid[(localPlayer->y - F10) >> 15]
             [(localPlayer->x + F26) >> 15].event == 126) &&
        checkMask(localPlayer->x + F30, localPlayer->y - F10)  )      )) {

    localPlayer->energy--;

    if (localPlayer->energy) {

      localPlayer->reaction = PR_HURT;
      localPlayer->reactionTime = ticks + 1000;

      if (localPlayer->dx < 0) {

        localPlayer->dx = PS_RUN;
        localPlayer->dy = PS_JUMP;

      } else {

        localPlayer->dx = -PS_RUN;
        localPlayer->dy = PS_JUMP;

      }

    } else {

      localPlayer->reaction = PR_KILLED;
      localPlayer->reactionTime = ticks + 1000;

    }

  }


  // Handle firing

  if (localPlayer->controls[C_FIRE] == SDL_PRESSED) {

    if (ticks > localPlayer->fireTime) {

      createPlayerBullet(localPlayer, ticks);
      if (localPlayer->fireSpeed)
        localPlayer->fireTime = ticks + (1000 / localPlayer->fireSpeed);
      else localPlayer->fireTime = 0x7FFFFFFF;

    }

  } else localPlayer->fireTime = 0;


  // Process bullets

  bul = firstBullet;
  prevBul = NULL;

  while (bul) {

    if ((ticks > bul->time) ||
        (checkMask(bul->x, bul->y) &&
         (bul->type != B_BOUNCER) && (bul->type != B_TNT))) {

      if (bul->type == B_TNT) {

        for (y = (bul->y - F160) >> 15; y < (bul->y + F160) >> 15; y++) {

          for (x = (bul->x - F100) >> 15; x < (bul->x + F100) >> 15; x++) {

            if ((y >= 0) && (y < 64) && (x >= 0) && (x < 256)) {

              // If the event is killable, remove it
              if (eventSet[grid[y][x].event][E_HITSTOKILL])
                grid[y][x].event = 0;

            }

          }

        }

      }

      bul = bul->next;

      // Hit an obstacle or time expired, destroy the bullet
      removeBullet(prevBul);

    } else {

      if (bul->type == B_BOUNCER) {

        if (checkMaskDown(bul->x, bul->y - 4) && (bul->dy < 0)) bul->dy = 0;
        else if (checkMaskDown(bul->x, bul->y + 4)) bul->dy = -600 * F1;
        else if (checkMaskDown(bul->x - 4, bul->y)) bul->dx = 500 * F1;
        else if (checkMaskDown(bul->x + 4, bul->y)) bul->dx = -500 * F1;
        else bul->dy += 3200 * mspf;

      }

      bul->x += (bul->dx * mspf) >> 10;
      bul->y += (bul->dy * mspf) >> 10;

      prevBul = bul;
      bul = bul->next;

    }

  }


  // Check for a change in ammo
  if (controls[C_CHANGE].state == SDL_PRESSED) {

    releaseControl(C_CHANGE);

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


  // Check if time has run out
  if ((ticks > endTicks) && (localPlayer->reaction != PR_KILLED) &&
      (localPlayer->reaction != PR_WON) &&
      (difficulty >= 2)) {

    localPlayer->reaction = PR_KILLED;
    localPlayer->reactionTime = endTicks + 2000;

  }


  return SUCCESS;

}



void drawLevel (int ticks) {

  gridElement *ge;
  bullet *bul;
  event *evt;
  SDL_Rect src, dst;
  int x, y, frame;


  // The current frame for animations
  frame = ticks / 75;


  // Choose player animation

  if (localPlayer->reaction == PR_KILLED)
    localPlayer->anim = localPlayer->anims[PA_LDIE + localPlayer->facing];

  else if ((localPlayer->reaction == PR_HURT) &&
           (localPlayer->reactionTime - ticks > 800))
    localPlayer->anim = localPlayer->anims[PA_LHURT + localPlayer->facing];

  else if (localPlayer->floating)
    localPlayer->anim = localPlayer->anims[PA_LBOARD + localPlayer->facing];

  else if (localPlayer->dy >= 0) {

    if ((eventSet[localPlayer->event][E_MODIFIER] == 6) ||
        (eventSet[localPlayer->event][E_BEHAVIOUR] == 28) ||
        checkMaskDown(localPlayer->x + F12, localPlayer->y + F4) ||
        checkMaskDown(localPlayer->x + F16, localPlayer->y + F4) ||
        checkMaskDown(localPlayer->x + F20, localPlayer->y + F4) ||
        checkMaskDown(localPlayer->x + F12, localPlayer->y + F12) ||
        checkMaskDown(localPlayer->x + F16, localPlayer->y + F12) ||
        checkMaskDown(localPlayer->x + F20, localPlayer->y + F12)   ) {

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

        if (!checkMaskDown(localPlayer->x + F12, localPlayer->y + F12) &&
            !checkMaskDown(localPlayer->x + F8, localPlayer->y + F8) &&
            (eventSet[localPlayer->event][E_MODIFIER] != 6) &&
            (eventSet[localPlayer->event][E_BEHAVIOUR] != 28)            )
          localPlayer->anim = localPlayer->anims[PA_LEDGE];
        else if (!checkMaskDown(localPlayer->x + F20, localPlayer->y + F12) &&
                 !checkMaskDown(localPlayer->x + F24, localPlayer->y + F8) &&
                 (eventSet[localPlayer->event][E_MODIFIER] != 6) &&
                 (eventSet[localPlayer->event][E_BEHAVIOUR] != 28)           )
          localPlayer->anim = localPlayer->anims[PA_REDGE];
        else if (localPlayer->controls[C_FIRE] == SDL_PRESSED)
          localPlayer->anim = localPlayer->anims[PA_LSHOOT +
                                                 localPlayer->facing];
      else if ((localPlayer->lookTime < 0) &&
                 (ticks > 1000 - localPlayer->lookTime))
          localPlayer->anim = localPlayer->anims[PA_LOOKUP];
        else if (localPlayer->lookTime > 0) {
          if (ticks < 1000 + localPlayer->lookTime)
            localPlayer->anim = localPlayer->anims[PA_LCROUCH +
                                                   localPlayer->facing];
          else localPlayer->anim = localPlayer->anims[PA_LOOKDOWN];
        } else
          localPlayer->anim = localPlayer->anims[PA_LSTAND +
                                                 localPlayer->facing];

      }

    } else localPlayer->anim = localPlayer->anims[PA_LFALL +
                                                  localPlayer->facing];

  } else if (eventSet[localPlayer->event][E_MODIFIER] == 29)
    localPlayer->anim = localPlayer->anims[PA_LSPRING - localPlayer->facing];
    
  else localPlayer->anim = localPlayer->anims[PA_LJUMP + localPlayer->facing];


  // Set tile drawing dimensions
  src.w = TW;
  src.h = TH;
  src.x = 0;


  // Use the player's viewport
  dst.x = 0;
  dst.y = 0;
  dst.w = localPlayer->viewW;
  dst.h = localPlayer->viewH;
  SDL_SetClipRect(screen, &dst);


  // If there is a sky, draw it
  if (bgPE->type == PE_SKY) {

    dst.x = 0;
    dst.w = screenW;
    dst.h = bgScale;

    dst.y = 0;
    SDL_FillRect(screen, &dst, 156);

    for (y = ((localPlayer->viewY >> 10) % bgScale);
         y < localPlayer->viewH; y += bgScale       ) {

      dst.y = y;
      SDL_FillRect(screen, &dst, 157 + (y / bgScale));

    }

    // Assign the correct portion of the sky palette
    bgPE->position = localPlayer->viewY + (localPlayer->viewH << 9) - F4;

    // Show sun / moon / etc.
    if (skyOrb) {

      dst.x = (localPlayer->viewW * 4) / 5;
      dst.y = (localPlayer->viewH * 3) / 25;
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

    ((short int *)&(bgPE->position))[0] = localPlayer->viewX >> 10;
    ((short int *)&(bgPE->position))[1] = localPlayer->viewY >> 10;

  }


  // Show background tiles

  for (y = 0; y <= ((localPlayer->viewH - 1) >> 5) + 1; y++) {

    for (x = 0; x <= ((localPlayer->viewW - 1) >> 5) + 1; x++) {

      // Get the grid element from the given coordinates
      ge = grid[y + (localPlayer->viewY >> 15)] + x +
           (localPlayer->viewX >> 15);

      // If this tile uses a black background, draw it
      if (ge->bg) {

        dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
        dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
        dst.w = dst.h = TW;
        SDL_FillRect(screen, &dst, 31);

      }

      // If this is not a foreground tile, draw it
      if ((eventSet[ge->event][E_BEHAVIOUR] != 38) &&
          ((ge->event < 124) || (ge->event > 125))  ) {

        dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
        dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
        src.y = ge->tile << 5;
        SDL_BlitSurface(tileSet, &src, screen, &dst);

      }

    }

  }


  // Show active events
  evt = firstEvent;

  while (evt) {

    drawEvent(evt, ticks);
    evt = evt->next;

  }


  // Show the player

  if ((localPlayer->reaction == PR_HURT) && (!((ticks / 30) & 3)))
    scalePalette(animSet[localPlayer->anim].
                  sprites[frame % animSet[localPlayer->anim].frames].pixels,
                 0, 36                                                      );

  dst.x = ((localPlayer->x - localPlayer->viewX) >> 10) +
          animSet[localPlayer->anim].
          sprites[frame % animSet[localPlayer->anim].frames].x;
  dst.y = ((localPlayer->y + F4 - localPlayer->viewY) >> 10) +
          animSet[localPlayer->anim].
          sprites[frame % animSet[localPlayer->anim].frames].y +
          animSet[localPlayer->anim].
          y[frame % animSet[localPlayer->anim].frames] -
          animSet[localPlayer->anim].sprites[0].pixels->h;
  SDL_BlitSurface(animSet[localPlayer->anim].
                  sprites[frame % animSet[localPlayer->anim].frames].
                  pixels, NULL, screen, &dst);

  if ((localPlayer->reaction == PR_HURT) && (!((ticks / 30) & 3)))
      restorePalette(animSet[localPlayer->anim].
                     sprites[frame % animSet[localPlayer->anim].frames].
                     pixels                                             );

  // Show invincibility stars

  if (localPlayer->reaction == PR_INVINCIBLE) {

    dst.x = (localPlayer->x + F6 - localPlayer->viewX) >> 10;
    dst.y = (localPlayer->y - F32 - localPlayer->viewY) >> 10;
    SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                    NULL, screen, &dst);

    dst.x = (localPlayer->x + F16 - localPlayer->viewX) >> 10;
    dst.y = (localPlayer->y - F32 - localPlayer->viewY) >> 10;
    SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                    NULL, screen, &dst);

    dst.x = (localPlayer->x + F6 - localPlayer->viewX) >> 10;
    dst.y = (localPlayer->y - F16 - localPlayer->viewY) >> 10;
    SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                    NULL, screen, &dst);

    dst.x = (localPlayer->x + F16 - localPlayer->viewX) >> 10;
    dst.y = (localPlayer->y - F16 - localPlayer->viewY) >> 10;
    SDL_BlitSurface(animSet[122].sprites[frame % animSet[122].frames].pixels,
                    NULL, screen, &dst);

  }


  // Show bullets

  bul = firstBullet;

  while (bul) {

    dst.x = (bul->x >> 10) - (spriteSet[bul->type].pixels->w >> 1) -
            (localPlayer->viewX >> 10);
    dst.y = (bul->y >> 10) - (spriteSet[bul->type].pixels->h >> 1) -
            (localPlayer->viewY >> 10);

    // Show the bullet
    SDL_BlitSurface(spriteSet[bul->type].pixels, NULL, screen, &dst);

    bul = bul->next;

  }


  // Show foreground tiles

  for (y = 0; y <= ((localPlayer->viewH - 1) >> 5) + 1; y++) {

    for (x = 0; x <= ((localPlayer->viewW - 1) >> 5) + 1; x++) {

      // Get the grid element from the given coordinates
      ge = grid[y + (localPlayer->viewY >> 15)] + x +
           (localPlayer->viewX >> 15);

      // If this is an "animated" foreground tile, draw it
      if (ge->event == 123) {

        dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
        dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
        if (frame & 1) src.y = eventSet[ge->event][E_YAXIS] << 5;
        else src.y = eventSet[ge->event][E_MULTIPURPOSE] << 5;
        SDL_BlitSurface(tileSet, &src, screen, &dst);

      }

      // If this is a foreground tile, draw it
      if ((ge->event == 124) || (ge->event == 125) ||
          (eventSet[ge->event][E_BEHAVIOUR] == 38)   ) {

        dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
        dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
        src.y = ge->tile << 5;
        SDL_BlitSurface(tileSet, &src, screen, &dst);

      }

    }

  }

  SDL_SetClipRect(screen, NULL);

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

  // Change the ammo type display on the panel
  dst.x = 250;
  dst.y = 2;
  SDL_BlitSurface(panelAmmo[localPlayer->ammoType + 1], NULL, panel, &dst);

  dst.x = 0;
  dst.y = screenH - 33;
  SDL_BlitSurface(panel, NULL, screen, &dst);
  dst.y += 32;
  dst.w = 320;
  dst.h = 1;
  SDL_FillRect(screen, &dst, 31);


  // Show panel data

  // Show score
  showNumber(localPlayer->score, 84, screenH - 27, &panelSmallFont);

  // Show time remaining
  if (endTicks > ticks) x = endTicks - ticks;
  else x = 0;
  y = x / (60 * 1000);
  showNumber(y, 116, screenH - 27, &panelSmallFont);
  x -= (y * 60 * 1000);
  y = x / 1000;
  showNumber(y, 136, screenH - 27, &panelSmallFont);
  x -= (y * 1000);
  y = x / 100;
  showNumber(y, 148, screenH - 27, &panelSmallFont);

  // Show lives
  showNumber(localPlayer->lives, 124, screenH - 13, &panelSmallFont);

  // Show planet number
  if (world <= 41) // Main game levels
    showNumber((world % 3) + 1, 184, screenH - 13, &panelSmallFont);
  else if ((world >= 50) && (world <= 52)) // Christmas levels
    showNumber(world - 49, 184, screenH - 13, &panelSmallFont);
  else // New levels
    showNumber(world, 184, screenH - 13, &panelSmallFont);

  // Show level number
  showNumber(level + 1, 196, screenH - 13, &panelSmallFont);

  // Show ammo
  if (localPlayer->ammoType == -1)
    showString(":;", 224, screenH - 13, &panelSmallFont);
  else showNumber(localPlayer->ammo[localPlayer->ammoType], 244, screenH - 13,
                  &panelSmallFont);

  // Draw the health bar

  dst.x = 20;
  dst.y = screenH - 13;
  dst.h = 7;

  if ((localPlayer->energyBar >> 10) < (localPlayer->energy << 4)) {

    if ((localPlayer->energy << 14) - localPlayer->energyBar < mspf * 40)
      localPlayer->energyBar = localPlayer->energy << 14;
    else localPlayer->energyBar += mspf * 40;

  } else if ((localPlayer->energyBar >> 10) > (localPlayer->energy << 4)) {

    if (localPlayer->energyBar - (localPlayer->energy << 14) < mspf * 40)
      localPlayer->energyBar = localPlayer->energy << 14;
    else localPlayer->energyBar -= mspf * 40;

  }

  if (localPlayer->energyBar > F1) {

    dst.w = (localPlayer->energyBar >> 10) - 1;

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


  return;

}



int runLevel (char * fn) {

  char *options[5] = {"CONTINUE GAME",
                      "SAVE GAME",
                      "LOAD GAME",
                      "SETUP OPTIONS",
                      "QUIT GAME"};
  SDL_Rect dst;
  float smoothfps;
  int tickOffset, prevTicks, ticks, count, paused, menu, option;


  if (loadLevel(fn)) return FAILURE;

  // Arbitrary initial value
  smoothfps = 50.0f;

  tickOffset = SDL_GetTicks();
  ticks = -10;

  option = menu = paused = 0;

  while (1) {

    // Do general processing
    if (loop() == QUIT) {

      freeLevel();
      free(sceneFile);

      return QUIT;

    }

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      paused = !paused;
      menu = !menu;
      option = 0;

    }

    if (controls[C_PAUSE].state == SDL_PRESSED) {

      releaseControl(C_PAUSE);

      paused = !paused;

    }

    if (controls[C_STATS].state == SDL_PRESSED) {

      releaseControl(C_STATS);

      stats = (stats + 1) & 3;

    }

    if (menu) {

      // Deal with menu controls

      if (controls[C_UP].state == SDL_PRESSED) {

        releaseControl(C_UP);

        option = (option + 4) % 5;

      }

      if (controls[C_DOWN].state == SDL_PRESSED) {

        releaseControl(C_DOWN);

        option = (option + 1) % 5;

      }

      if (controls[C_ENTER].state == SDL_PRESSED) {

        releaseControl(C_ENTER);

        switch (option) {

          case 0: // Continue

            paused = !paused;
            menu = !menu;

            break;

          case 1: // Save

            break;

          case 2: // Load

            break;

          case 3: // Setup

            if (setupMenuLoop() == QUIT) {

              freeLevel();
              free(sceneFile);

              return QUIT;

            }

            // Restore level palette
            usePalette(levelPalette);

            // Ensure player name string is still valid
            localPlayer->name = localPlayerName;

            break;

          case 4: // Quit

            freeLevel();
            free(sceneFile);

            return SUCCESS;

        }

      }

    }


    if (fps) {

      smoothfps = smoothfps + 1 - (smoothfps / fps);
      /* This equation is a simplified version of
         (fps * c) + (smoothfps * (1 - c))
         where c = (1 / fps)
         In other words, the response of smoothFPS to changes in FPS
         decreases as the framerate increases 
         The following version is for c = (1 / smoothfps)
         //      smoothfps = (fps / smoothfps) + smoothfps - 1;
         I bet about a week after I came up with this, I'll find out that it's
         been around for years. */

      // Ignore outlandish values
      if (smoothfps > 9999) smoothfps = 9999;
      if (smoothfps < 1) smoothfps = 1;

    }

    // Number of ticks of gameplay since the level started

    prevTicks = ticks;
    ticks = SDL_GetTicks() - tickOffset;

    if (paused) {

      tickOffset += ticks - prevTicks;
      ticks = prevTicks;

    } else if (ticks > prevTicks + 100) {

      tickOffset += ticks - (prevTicks + 100);
      ticks = prevTicks + 100;

    }

    // Process frame-by-frame activity

    if (!paused) {

      if (playLevelFrame(ticks) == FAILURE) return FAILURE;

      // Handle player reactions
      if (localPlayer->reaction && (localPlayer->reactionTime < ticks)) {

        switch (localPlayer->reaction) {

          case PR_KILLED:

            localPlayer->lives--;

            freeLevel();
            free(sceneFile);

            if (!localPlayer->lives) return SUCCESS; // Not really a success...

            dst.x = checkX;
            dst.y = checkY;

            if (loadLevel(currentLevel)) return FAILURE;

            localPlayer->x = dst.x << 15;
            localPlayer->y = dst.y << 15;

            break;

          case PR_WON:

            if (!strcmp(nextLevel, "endepis")) {

              freeLevel();
              runScene(sceneFile);
              free(sceneFile);

              return SUCCESS;

            }

            freeLevel();
            free(sceneFile);

            if (loadLevel(nextLevel)) return FAILURE;

            break;

        }

        localPlayer->reaction = PR_NONE;

      }

    }


    // Draw the graphics

    if ((localPlayer->viewW < screenW) || (localPlayer->viewH < screenH))
      SDL_FillRect(screen, NULL, 15);

    drawLevel(ticks);


    if (paused != menu) showString("PAUSE", (screenW >> 1) - 44, 32, fontmn1);


    // Draw players/statistics

    if (stats & S_PLAYERS) {

      dst.x = 128;
      dst.y = 11;
      dst.w = 96;

      for (count = 0; count < nPlayers; count++)
        if ((strlen(players[count].name) * 8) + 33 > dst.w)
          dst.w = (strlen(players[count].name) * 8) + 33;

      dst.h = (nPlayers * 12) + 1;
      SDL_FillRect(screen, &dst, 31);

      for (count = 0; count < nPlayers; count++) {

        showNumber(count + 1, 152, 14 + (count * 12), &panelBigFont);
        showString(players[count].name, 160, 14 + (count * 12), &panelBigFont);

      }

    }

    if (stats & S_SCREEN) {

      dst.x = 236;
      dst.y = 9;
      dst.w = 80;
      dst.h = 32;
      SDL_FillRect(screen, &dst, 31);

      showNumber(screenW, 268, 15, &panelBigFont);
      showString("x", 272, 15, &panelBigFont);
      showNumber(screenH, 308, 15, &panelBigFont);
      showString("fps", 244, 27, &panelBigFont);
      showNumber(smoothfps, 308, 27, &panelBigFont);

    }

    if (menu) {

      // Draw the menu

      dst.x = (screenW >> 2) - 8;
      dst.y = (screenH >> 1) - 46;
      dst.w = 144;
      dst.h = 92;
      SDL_FillRect(screen, &dst, 31);

      for (count = 0; count < 5; count++) {

        if (count == option)
          scalePalette(fontmn2->pixels, -F2, (-240 * -2) + 48);
        else scalePalette(fontmn2->pixels, -F2, (-240 * -2) + 16);

        showString(options[count], screenW >> 2,
                   (screenH >> 1) + (count << 4) - 38, fontmn2);

      }

      restorePalette(fontmn2->pixels);

    }

  }

  return SUCCESS;

}




