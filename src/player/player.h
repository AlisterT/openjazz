
/*
 *
 * player.h
 *
 * 31st January 2006: Created player.h from parts of OpenJazz.h
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
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


#ifndef _PLAYER_H
#define _PLAYER_H


#include "movable.h"
#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Level animations
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

// Bonus level animations
#define PA_WALK  0
#define PA_LEFT  1
#define PA_RIGHT 2
#define PA_USER  3
#define PA_JUMP  4
#define PA_CRASH 5
#define PA_OTHER 6

// Player reaction times
#define PRT_HURT       1000
#define PRT_HURTANIM   200
#define PRT_KILLED     2000
#define PRT_INVINCIBLE 10000

// Other time periods
#define T_FASTFEET 25000
#define T_WARP     1000

// Player offsets
#define PXO_MID F16
#define PXO_L   (PXO_MID - F10)
#define PXO_ML  (PXO_MID - F4)
#define PXO_MR  (PXO_MID + F4)
#define PXO_R   (PXO_MID + F10)
#define PYO_TOP (-F20)
#define PYO_MID (-F10)

// Player speeds
#define PXS_WALK  ITOF(300)
#define PXS_RUN   ITOF(325)
#define PXS_FFRUN ITOF(500)
#define PYS_FALL  ITOF(350)
#define PYS_SINK  ITOF(150)
#define PYS_JUMP  ITOF(-350)

#define PRS_REVERSE ITOF(-32)
#define PRS_RUN     ITOF(64)
#define PRS_ROLL    ITOF(96)

// Player accelerations
#define PXA_REVERSE 900
#define PXA_STOP    1000
#define PXA_WALK    500
#define PXA_RUN     200
#define PXA_FFRUN   200
#define PYA_GRAVITY 2750
#define PYA_SINK    1000

#define PRA_REVERSE 450
#define PRA_STOP    500
#define PRA_RUN     100

// Player colours
#define PC_WHITE     0
#define PC_SGREEN    16
#define PC_BLUE      23
#define PC_RED       32
#define PC_LGREEN    48
#define PC_LEVEL1    64
#define PC_YELLOW    75
#define PC_LEVEL2    80
#define PC_ORANGE    88
#define PC_LEVEL3    96
#define PC_LEVEL4    104
#define PC_SANIM     112
#define PC_LANIM     116
#define PC_LEVEL5    124

// Player defaults
#define CHAR_NAME   "jazz"
#define CHAR_FUR    PC_LGREEN
#define CHAR_BAND   PC_RED
#define CHAR_GUN    PC_BLUE
#define CHAR_WBAND  PC_ORANGE

// General
#define PANIMS     38 /* Number of player animations. Is probably higher. */
#define PCONTROLS   8 /* Number of player controls. */


// Enum

enum PlayerReaction {

	PR_NONE, PR_HURT, PR_KILLED, PR_INVINCIBLE

};


// Classes

class Anim;
class Bird;
class Bonus;

class Player : public Movable {

	private:
		Bird*           bird;
		char*           name;
		char            anims[PANIMS];
		bool            pcontrols[PCONTROLS];
		SDL_Color       palette[256];
		unsigned char   cols[4];
		int             ammo[4];
		int             ammoType; /* -1 = blaster, 0 = toaster, 1 = missiles, 2 = bouncer 3 = TNT */
		int             score;
		int             energy;
		int             lives;
		int             shield; /* 0 = none, 1 = 1 yellow, 2 = 2 yellow, 3 = 1 orange, 4 = 2 orange, 5 = 3 orange, 6 = 4 orange */
		bool            floating; /* false = normal, true = boarding/bird/etc. */
		bool            facing;
		fixed           direction;
		unsigned char   animType;
		unsigned char   eventX;
		unsigned char   eventY; /* Position of an event (spring, platform, bridge) */
		int             event;  /* 0 = none, 1 = spring, 2 = float up, 3 = platform, 4 = bridge */
		int             lookTime; /* Negative if looking up, positive if looking down, 0 if neither */
		PlayerReaction  reaction;
		unsigned int    reactionTime;
		int             fireSpeed;
		unsigned int    fireTime;
		fixed           jumpHeight;
		fixed           jumpY;
		unsigned int    fastFeetTime;
		unsigned char   warpX, warpY;
		unsigned int    warpTime;
		int             enemies, items;
		unsigned char   team;
		bool            gem;

		void addAmmo (int type, int amount);

	public:
		int teamScore;

		Player                       ();
		~Player                      ();

		void            init         (char* playerName, unsigned char* cols, unsigned char newTeam);
		void            deinit       ();
		void            setAnims     (char* newAnims);
		char*           getName      ();
		unsigned char*  getCols      ();
		void            reset        ();
		void            setControl   (int control, bool state);
		bool            takeEvent    (unsigned char gridX, unsigned char gridY, unsigned int ticks);
		bool            touchEvent   (unsigned char gridX, unsigned char gridY, unsigned int ticks, int msps);
		void            setEvent     (unsigned char gridX, unsigned char gridY);
		void            clearEvent   (unsigned char gridX, unsigned char gridY);
		bool            hit          (Player* source, unsigned int ticks);
		void            kill         (Player* source, unsigned int ticks);
		void            addItem      ();
		void            addLife      ();
		void            addScore     (int addedScore);
		int             getScore     ();
		int             getEnergy    ();
		int             getLives     ();
		int             getAmmo      (bool amount);
		int             getEnemies   ();
		int             getItems     ();
		bool            overlap      (fixed left, fixed top, fixed width, fixed height);
		void            setPosition  (fixed newX, fixed newY);
		void            setSpeed     (fixed newDx, fixed newDy);
		bool            getFacing    ();
		fixed           getDirection ();
		unsigned char   getAnim      ();
		unsigned char   getTeam      ();
		bool            hasGem       ();
		void            send         (unsigned char* data);
		void            receive      (unsigned char* buffer);
		void            control      (unsigned int ticks, int msps);
		void            move         (unsigned int ticks, int msps);
		void            bonusStep    (unsigned int ticks, int msps, Bonus* bonus);
		void            view         (unsigned int ticks, int mspf);
		void            draw         (unsigned int ticks, int change);
		PlayerReaction  reacted      (unsigned int ticks);

};


// Variables

EXTERN Player* players;
EXTERN Player* localPlayer;
EXTERN int     nPlayers;

// Configuration data
EXTERN char*         characterName;
EXTERN unsigned char characterCols[4];

#endif

