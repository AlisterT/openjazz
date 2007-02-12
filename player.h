
/*
 *
 * player.h
 * Created on the 31st of January 2006
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


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#include "OpenJazz.h"

#ifndef Lextern
  #define Lextern extern
#endif


// Constants

// Player animations
#define PA_LWALK    0
#define PA_RWALK    1
#define PA_LJUMP    2
#define PA_RJUMP    3
#define PA_LSPIN    4
#define PA_RSPIN    5
#define PA_LSHOOT   6
#define PA_RSHOOT   7
#define PA_LCROUCH  8
#define PA_RCROUCH  9
#define PA_LFALL    10
#define PA_RFALL    11
#define PA_LHURT    12
#define PA_RHURT    13
#define PA_LLEAN    14
#define PA_RLEAN    15
#define PA_LBOARD   16
#define PA_RBOARD   17
#define PA_LSTAND   18
#define PA_RSTAND   19
#define PA_LEAT     20
#define PA_REAT     21
#define PA_LEDGE    22
#define PA_REDGE    23
#define PA_LOOKUP   24
#define PA_LOOKDOWN 25
#define PA_LSWIM    26
#define PA_RSWIM    27
#define PA_LRUN     28
#define PA_RRUN     29
#define PA_LDIE     30
#define PA_RDIE     31
#define PA_LSTOP    32
#define PA_RSTOP    33
#define PA_LHALT    34 /* Yeah, I was wondering the same thing... */
#define PA_RHALT    35
#define PA_RSPRING  36
#define PA_LSPRING  37 /* Surely these are the wrong way round? */

// Player facing
#define PF_LEFT  0
#define PF_RIGHT 1

// Player reactions
#define PR_NONE       0
#define PR_HURT       1
#define PR_KILLED     2
#define PR_INVINCIBLE 3
#define PR_WON        4

// Player speeds
#define PS_WALK (300 * F1)
#define PS_RUN  (325 * F1)
#define PS_FALL (350 * F1)
#define PS_JUMP (-350 * F1)

// General
#define PANIMS     38 /* Number of player animations. Is probably higher. */
#define PCONTROLS   7 /* Number of player controls. */


// Datatype

typedef struct {

    char  *name;
    char   anims[PANIMS];
    int    anim;
    int    facing;
    fixed  viewX, viewY;
    int    viewW, viewH;
    fixed  x, y;
    fixed  dx, dy;
    int    jumpHeight;
    int    jumpY;
    int    score;
    int    energy;
    fixed  energyBar;
    int    lives;
    int    reaction;
    int    reactionTime;
    int    ammo[4];
    int    ammoType; /* -1 = blaster, 0 = toaster, 1 = missiles, 2 = bouncer
                       3 = TNT */
    int    shield; /* 0 = none, 1 = 1 yellow, 2 = 2 yellow, 3 = 1 orange,
                     4 = 2 orange, 5 = 3 orange, 6 = 4 orange */
    int    floating; // 0 = normal, 1 = boarding/birding/whatever
    int    event; // Indexes the event set (spring, float up, belt, platform)
    Uint8  controls[PCONTROLS];
    int    fireSpeed;
    int    fireTime;
    int    lookTime; /* Negative if looking up, positive if looking down, 0 if
                        neither */

} player;


// Variables

Lextern player        *players;
Lextern int            nPlayers;
Lextern player        *localPlayer;

