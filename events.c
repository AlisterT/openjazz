
/*
 *
 * events.c
 * Created on the 1st of January 2006
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2006 Alister Thomson
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
 * Deals with events in ordinary levels.
 *
 */


#include "OpenJazz.h"


void createPlayerBullet (player *source, int ticks) {

  bullet *bul;

  if ((source->ammoType == -1) ||
      (source->ammo[source->ammoType])) {

    if (source->ammoType != -1)
      source->ammo[source->ammoType]--;

    if (unusedBullet) {

      bul = unusedBullet;
      unusedBullet = unusedBullet->next;

    } else bul = malloc(sizeof(bullet));

    bul->next = firstBullet;
    bul->origin = 0;

    switch (source->ammoType) {

      case -1:

        bul->type = B_BLASTER;

        break;

      case 0:

        if (source->facing) bul->type = B_RTOASTER;
        else bul->type = B_LTOASTER;

        break;

      case 1:

        if (source->facing) bul->type = B_RMISSILE;
        else bul->type = B_LMISSILE;

        break;

      case 2:

        bul->type = B_BOUNCER;

        break;

      case 3:

        bul->type = B_TNT;

        break;

    }

    bul->x = source->x + 6 + (20 * source->facing);
    bul->y = source->y - 8;

    if ((bul->type == B_LMISSILE) || (bul->type == B_RMISSILE)) {

      if (source->facing) {

        bul->dx = 500;
        bul->dy = -100;

      } else {

        bul->dx = -500;
        bul->dy = -100;

      }

      bul->time = ticks + 1000;
      firstBullet = bul;

      if (unusedBullet) {

        bul = unusedBullet;
        unusedBullet = unusedBullet->next;

      } else bul = malloc(sizeof(bullet));

      bul->next = firstBullet;
      bul->origin = 0;

      if (source->facing) bul->type = B_RMISSILE;
      else bul->type = B_LMISSILE;

      bul->x = source->x + 6 + (20 * source->facing);
      bul->y = source->y - 16;

      if (source->facing) {

        bul->dx = 500;
        bul->dy = 100;

      } else {

        bul->dx = -500;
        bul->dy = 100;

      }

    } else {

      if (bul->type == B_TNT) bul->dx = 0;
      else {

        if (!source->facing) {

          bul->dx = -500;

        } else {

          bul->dx = 500;

        }

      }

      bul->dy = 0;

    }

    bul->time = ticks + 1000;
    firstBullet = bul;

  }

  return;

}

void createEventBullet (event *evt, int ticks) {

  return;

}

void removeBullet (bullet * previous) {

  bullet *bul;

  if (previous) {

    bul = previous->next;
    previous->next = bul->next;

  } else {

    bul = firstBullet;
    firstBullet = bul->next;

  }

  // Submit for recycling
  bul->next = unusedBullet;
  unusedBullet = bul;

}


void freeBullets (void) {

  bullet *bul;

  while (firstBullet) {

    bul = firstBullet->next;
    free(firstBullet);
    firstBullet = bul;

  }

  while (unusedBullet) {

    bul = unusedBullet->next;
    free(unusedBullet);
    unusedBullet = bul;

  }

  return;

}


void createEvent (int x, int y) {

  event *evt;

  if (unusedEvent) {

    evt = unusedEvent;
    unusedEvent = unusedEvent->next;

  } else evt = malloc(sizeof(event));

  evt->gridX = x;
  evt->gridY = y;
  evt->x = x << 5;
  evt->y = (y + 1) << 5;
  evt->anim = 0;

  if (firstEvent) firstEvent->prev = evt;
  evt->next = firstEvent;
  evt->prev = NULL;
  firstEvent = evt;

  return;

}


void removeEvent (event * evt) {

  if (evt->prev) {

    evt->prev->next = evt->next;

    if (evt->next) evt->next->prev = evt->prev;

  } else {

    firstEvent = evt->next;

    if (firstEvent) firstEvent->prev = NULL;

  }

  // Submit for recycling
  evt->next = unusedEvent;
  unusedEvent = evt;

  return;

}


void freeEvents (void) {

  event *evt;

  while (firstEvent) {

    evt = firstEvent->next;
    free(firstEvent);
    firstEvent = evt;

  }

  while (unusedEvent) {

    evt = unusedEvent->next;
    free(unusedEvent);
    unusedEvent = evt;

  }

  return;

}


void processEvent (event * evt, int ticks) {

  SDL_Rect pos;
  bullet *bul, *prevBul;
  gridElement *ge;
  unsigned char *set;
  int frame;

  // Get the grid element at the given coordinates
  ge = grid[evt->gridY] + evt->gridX;

  // Check whether or not the event should be processed
  if (!ge->event || (ge->event >= 122)) return;

  set = eventSet[ge->event];


  // Find frame and dimensions
  if (evt->anim && (set[evt->anim] < ANIMS)) {

    if (evt->anim == E_FINISHANIM)
      frame = ((ticks + 200 - ge->time) / 40) % animSet[set[evt->anim]].frames;
    else if (set[E_ANIMSP])
      frame = (ticks / (set[E_ANIMSP] * 40)) % animSet[set[evt->anim]].frames;
    else frame = (ticks / 40) % animSet[set[evt->anim]].frames;

    pos.x = (int)evt->x + animSet[set[evt->anim]].sprites[frame].x;
    pos.y = (int)evt->y + animSet[set[evt->anim]].sprites[frame].y +
            animSet[set[evt->anim]].y[frame] -
            animSet[set[evt->anim]].sprites[0].pixels->h;
    pos.w = animSet[set[evt->anim]].sprites[frame].pixels->w;
    pos.h = animSet[set[evt->anim]].sprites[frame].pixels->h;

    // Blank sprites
    if (pos.w == 1) pos.w = 0;
    if (pos.h == 1) pos.h = 0;

  } else {

    pos.x = evt->x;
    pos.y = evt->y - 32;
    pos.w = 32;
    pos.h = 32;

  }

  // Deal with bullet collisions
  if ((evt->anim != E_FINISHANIM) && (ge->hits < set[E_HITSTOKILL])) {

    bul = firstBullet;
    prevBul = NULL;

    while (bul) {

      if ((bul->x > pos.x) && (bul->x < pos.x + pos.w) &&
          (bul->y > pos.y) && (bul->y < pos.y + pos.h)   ) {

        ge->hits++;
        bul = bul->next;
        removeBullet(prevBul);

      } else {

        prevBul = bul;
        bul = bul->next;

      }

    }

  }


  // Handle behaviour
  if (evt->anim != E_FINISHANIM) {

    switch (set[E_BEHAVIOUR]) {

      case 0: // Face the player

        if (localPlayer->x + 16 < pos.x + (pos.w >> 1))
          evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        break;

      case 1: // Sink down

        if (localPlayer->x + 16 < pos.x + (pos.w >> 1))
          evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        evt->y += 200 * spf / set[E_MOVEMENTSP];

        break;

      case 2: // Walk from side to side

        // Walk from side to side
        if (evt->anim == E_LEFTANIM) {

          if (!checkMask(pos.x, pos.y + pos.h + 1) ||
              checkMask(pos.x - 1, pos.y + (pos.h >> 1)))
            evt->anim = E_RIGHTANIM;

          evt->x -= 200 * spf / set[E_MOVEMENTSP];

        } else if (evt->anim == E_RIGHTANIM) {

          if (!checkMask(pos.x + pos.w, pos.y + pos.h + 1) ||
              checkMask(pos.x + pos.w + 1, pos.y + (pos.h >> 1)))
            evt->anim = E_LEFTANIM;

          evt->x += 200 * spf / set[E_MOVEMENTSP];

        } else evt->anim = E_LEFTANIM;

        break;

      case 3: // Seek jazz

        if (localPlayer->x + 26 < pos.x) {

          evt->anim = E_LEFTANIM;
          evt->x -= 200 * spf;

        } else if (localPlayer->x + 6 > pos.x + pos.w) {

          evt->anim = E_RIGHTANIM;
          evt->x += 200 * spf;

        } else if (!evt->anim) evt->anim = E_RIGHTANIM;

        break;

      case 4: // Walk from side to side and down hills

        if (!checkMask(pos.x + (pos.w >> 1), pos.y + pos.h)) {

          // Fall downwards
          evt->y += 200 * spf / set[E_MOVEMENTSP];

        } else {

          // Walk from side to side
          if (evt->anim == E_LEFTANIM) {

            if (checkMask(pos.x - 1, pos.y + (pos.h >> 1)))
              evt->anim = E_RIGHTANIM;

            evt->x -= 200 * spf / set[E_MOVEMENTSP];

          } else if (evt->anim == E_RIGHTANIM) {

            if (checkMask(pos.x + pos.w + 1, pos.y + (pos.h >> 1)))
              evt->anim = E_LEFTANIM;

            evt->x += 200 * spf / set[E_MOVEMENTSP];

          } else evt->anim = E_LEFTANIM;

        }

        break;

      case 6: // Special behaviour

        // Temporary
        if (localPlayer->x < pos.x) evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;
        break;

      case 7: // Move back and forth horizontally with tail

        if (evt->anim == E_LEFTANIM) {

          evt->x -= 100 * spf;

          if (evt->x < evt->gridX << 5) evt->anim = E_RIGHTANIM;

        } else if (evt->anim == E_RIGHTANIM) {

          evt->x += 100 * spf;

          if (evt->x > (evt->gridX << 5) + 100) evt->anim = E_LEFTANIM;

        } else evt->anim = E_LEFTANIM;

        break;

      case 8: // Bird-esque following

        if (localPlayer->facing) {

          evt->anim = E_RIGHTANIM;

          if (evt->x < localPlayer->x + 64)
            evt->x += 200 * spf / set[E_MOVEMENTSP];

        } else {

          evt->anim = E_LEFTANIM;

          if (evt->x > localPlayer->x - 64)
            evt->x -= 200 * spf / set[E_MOVEMENTSP];

        }

        if (evt->y > localPlayer->y - 64)
          evt->y -= 200 * spf / set[E_MOVEMENTSP];
        else evt->y += 200 * spf / set[E_MOVEMENTSP];

        break;

      case 11: // Sink to ground

        if (localPlayer->x + 16 < pos.x + (pos.w >> 1))
          evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;

        if (!checkMask(pos.x + (pos.w >> 1), pos.y))
          evt->y += 200 * spf / set[E_MOVEMENTSP];

        break;

      case 12: // Move back and forth horizontally

        if (evt->anim == E_LEFTANIM) {

          if (checkMask(pos.x - 1, pos.y + (pos.h >> 1)))
            evt->anim = E_RIGHTANIM;

          evt->x -= 100 * spf;

        } else if (evt->anim == E_RIGHTANIM) {

          if (checkMask(pos.x + pos.w + 1, pos.y + (pos.h >> 1)))
            evt->anim = E_LEFTANIM;

          evt->x += 100 * spf;

        } else evt->anim = E_LEFTANIM;

        break;

      case 13: // Move up and down

        if (evt->anim == E_LEFTANIM) {

          if (checkMask(pos.x + (pos.w >> 1), pos.y - 1))
            evt->anim = E_RIGHTANIM;

          evt->y -= 100 * spf;

        } else if (evt->anim == E_RIGHTANIM) {

          if (checkMask(pos.x + (pos.w >> 1), pos.y + pos.h + 1))
            evt->anim = E_LEFTANIM;

          evt->y += 100 * spf;

        } else evt->anim = E_LEFTANIM;

        break;

      case 14: // Move back and forth rapidly

        // Temporary
        if (localPlayer->x < pos.x) evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;
        break;

      case 15: // Rise or lower to meet jazz

        // Temporary
        if (localPlayer->x < pos.x) evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;
        break;

      case 16: // Move across level to the left or right

        evt->x += ((signed char *)set)[E_MAGNITUDE] *
                  80 * spf / set[E_MOVEMENTSP];
        evt->anim = E_LEFTANIM;

        break;

      case 21: // Destructible block

        if (ge->hits >= set[E_HITSTOKILL]) ge->tile = set[E_MULTIPURPOSE];
        break;

      case 22: // Fall down in random spot and repeat
      case 24: // Crawl along ground and go downstairs

        // Temporary
        if (localPlayer->x < pos.x) evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;
        break;

      case 26: // Flip animation

        evt->anim = E_RIGHTANIM;

        break;

      case 27: // Face jazz
      case 28: // Bridge
      case 29: // Nonmoving object with jazz
      case 30: // Nonmoving object with jazz
      case 31: // Nonmoving object (no)
      case 32: // Nonmoving object

        // Temporary
        if (localPlayer->x < pos.x) evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;
        break;

      case 33: // Sparks-esque following

        if (localPlayer->facing && (pos.x + pos.w < localPlayer->x)) {

          evt->x += 200 * spf / set[E_MOVEMENTSP];
          evt->anim = E_RIGHTANIM;

          if (pos.y + pos.h < localPlayer->y - 20)
            evt->y += 40 * spf / set[E_MOVEMENTSP];

          else if (pos.y > localPlayer->y)
            evt->y -= 40 * spf / set[E_MOVEMENTSP];

        } else if (!localPlayer->facing && (pos.x > localPlayer->x + 32)) {

          evt->x += -200 * spf / set[E_MOVEMENTSP];
          evt->anim = E_LEFTANIM;

          if (pos.y + pos.h < localPlayer->y - 20)
            evt->y += 40 * spf / set[E_MOVEMENTSP];

          else if (pos.y > localPlayer->y)
            evt->y -= 40 * spf / set[E_MOVEMENTSP];

        } else if (!evt->anim) evt->anim = E_LEFTANIM;

        break;

      case 34: // Launching platform
      case 36: // Crawl along ground with jazz seek AI
      case 39: // Collapsing floor
      case 41: // Switch left & right anim periodically
      case 44: // Leap to greet Jazz very quickly
      case 46: // "Final" boss
      case 47:
      case 48:
      case 49:
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 56:
      case 59:
      case 60:
      case 61:
      case 62:
      case 65:

        // Temporary
        if (localPlayer->x < pos.x) evt->anim = E_LEFTANIM;
        else evt->anim = E_RIGHTANIM;
        break;

    }

  }


  // If the event has been destroyed, play its finishing animation and set its
  // reaction time
  if (set[E_HITSTOKILL] && (evt->anim != E_FINISHANIM) &&
     (ge->hits >= set[E_HITSTOKILL])                     ) {

    localPlayer->score += set[E_ADDEDSCORE] * 10;
    ge->time = ticks + 200;
    evt->anim = E_FINISHANIM;

  }


  // If the reaction time has expired
  if (ge->time && (ticks > ge->time)) {

    // Handle modifiers which take effect after reaction time
    switch (set[E_MODIFIER]) {

      case 41: // Bonus level

        nextlevel = set[E_MULTIPURPOSE];
        nextworld = set[E_YAXIS];

        // The lack of a break statement is intentional

      case 8: // Boss
      case 27: // End of level

        localPlayer->reaction = PR_WON;
        localPlayer->reactionTime = ticks + 1000;

        break;

      case 10:

        checkX = evt->gridX;
        checkY = evt->gridY;

        break;

      case 13: // Warp

        localPlayer->x = set[E_MULTIPURPOSE] << 5;
        localPlayer->y = set[E_YAXIS] << 5;
        ge->time = 0;

        break;

      case 15:

        if (!localPlayer->ammo[0]) localPlayer->ammoType = 0;
        localPlayer->ammo[0] += 15;
        break;

      case 16:

        if (!localPlayer->ammo[1]) localPlayer->ammoType = 1;
        localPlayer->ammo[1] += 15;
        break;

      case 17:

        if (!localPlayer->ammo[2]) localPlayer->ammoType = 2;
        localPlayer->ammo[2] += 15;
        break;

      case 34: // Bird

        ge->event = 121;
        removeEvent(evt);
        
        return;

    }

    if (evt->anim == E_FINISHANIM) {

      ge->event = 0;
      removeEvent(evt);

      return;

    } else {

      ge->time = 0;

    }

  }


  // Handle contact events
  if ((evt->anim != E_FINISHANIM) &&
      (localPlayer->x + 6 <= pos.x + pos.w) &&
      (localPlayer->x + 26 >= pos.x) &&
      (localPlayer->y - 20 <= pos.y + pos.h) &&
      (localPlayer->y >= pos.y)   ) {

    // Handle behaviour which takes effect on contact
    switch (set[E_BEHAVIOUR]) {

      case 26: // Flip animation

        evt->anim = E_LEFTANIM;

        break;

      case 25: // Float up

        localPlayer->dy -= set[E_MULTIPURPOSE] * 200 * spf;

        break;

      case 38: // Sucker tubes

        if (set[E_YAXIS]) localPlayer->dy = set[E_MULTIPURPOSE] * -20;

        if (set[E_MAGNITUDE] < 128) localPlayer->dx = set[E_MAGNITUDE] * 40;
        else localPlayer->dx = (set[E_MAGNITUDE] - 256) * 40;

        break;

    }

    // Handle modifiers which take effect on contact
    switch (set[E_MODIFIER]) {

      case 0: // Hurt
      case 8: // Boss

        if ((localPlayer->reaction == PR_NONE) &&
            ((set[E_BEHAVIOUR] < 37) || (set[E_BEHAVIOUR] > 44))) {

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

        break;

      case 1: // Invincibility

        localPlayer->reaction = PR_INVINCIBLE;
        localPlayer->reactionTime = ticks + 10000;

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 2:
      case 3: // Health

        if (localPlayer->energy < 4) localPlayer->energy++;

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 4: // extra life

        if (localPlayer->lives < 99) localPlayer->lives++;

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 5: // High-jump feet

        localPlayer->jumpDuration += 40;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 11: // Item

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 13: // Warp

        ge->time = ticks + 1000;

        break;

      case 18:

        if (!localPlayer->ammo[0]) localPlayer->ammoType = 0;
        localPlayer->ammo[0] += 2;
        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 19:

        if (!localPlayer->ammo[1]) localPlayer->ammoType = 1;
        localPlayer->ammo[1] += 2;
        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 20:

        if (!localPlayer->ammo[2]) localPlayer->ammoType = 2;
        localPlayer->ammo[2] += 2;
        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 29: // Upwards spring

        localPlayer->dy = (set[E_MAGNITUDE] - 256) * 80;

        break;

      case 35: // Airboard, etc.

        localPlayer->floating = 1;

        localPlayer->score += set[E_ADDEDSCORE] * 10;
        ge->time = ticks + 200;
        evt->anim = E_FINISHANIM;

        break;

      case 38: // Airboard, etc. off

        localPlayer->floating = 0;

        break;

      case 43: // Downwards spring

        localPlayer->dy = set[E_MAGNITUDE] * 80;

        break;

    }

  }

  // Show events
  // Note: 7 and 28 will need additional stuff
  if (evt->anim && (set[evt->anim] < ANIMS)) {

    if (evt->anim == E_FINISHANIM)
      frame = ((ticks + 200 - ge->time) / 40) % animSet[set[evt->anim]].frames;
    else if (set[E_ANIMSP])
      frame = (ticks / (set[E_ANIMSP] * 40)) % animSet[set[evt->anim]].frames;
    else frame = (ticks / 40) % animSet[set[evt->anim]].frames;

    pos.x = (int)evt->x - localPlayer->viewX +
            animSet[set[evt->anim]].sprites[frame].x;
    pos.y = (int)evt->y - localPlayer->viewY +
            animSet[set[evt->anim]].sprites[frame].y +
            animSet[set[evt->anim]].y[frame] -
            animSet[set[evt->anim]].sprites[0].pixels->h;

    // Draw the event
    SDL_BlitSurface(animSet[set[evt->anim]].sprites[frame].pixels, NULL, screen,
                    &pos);

  }

  return;

}

