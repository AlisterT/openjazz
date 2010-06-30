
/*
 *
 * jj2levelplayer.h
 *
 * 29th June 2010: Created jj2levelplayer.h from parts of levelplayer.h
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


#ifndef _JJ2LEVELPLAYER_H
#define _JJ2LEVELPLAYER_H


#include "player.h"

#include "level/movable.h"
#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Colour offsets
#define JJ2PCO_GREY      72
#define JJ2PCO_SGREEN    16
#define JJ2PCO_BLUE      35
#define JJ2PCO_RED       24
#define JJ2PCO_LGREEN    16
#define JJ2PCO_LEVEL1    96
#define JJ2PCO_YELLOW    59
#define JJ2PCO_LEVEL2    112
#define JJ2PCO_ORANGE    43
#define JJ2PCO_LEVEL3    128
#define JJ2PCO_LEVEL4    144
#define JJ2PCO_SANIM     160
#define JJ2PCO_LANIM     176
#define JJ2PCO_LEVEL5    208

// Colour lengths
#define JJ2PCL_GREY      8
#define JJ2PCL_SGREEN    8
#define JJ2PCL_BLUE      5
#define JJ2PCL_RED       8
#define JJ2PCL_LGREEN    8
#define JJ2PCL_LEVEL1    16
#define JJ2PCL_YELLOW    5
#define JJ2PCL_LEVEL2    16
#define JJ2PCL_ORANGE    5
#define JJ2PCL_LEVEL3    16
#define JJ2PCL_LEVEL4    16
#define JJ2PCL_SANIM     16
#define JJ2PCL_LANIM     32
#define JJ2PCL_LEVEL5    16

// Animations
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

// Player accelerations
#define PXA_REVERSE 900
#define PXA_STOP    1000
#define PXA_WALK    500
#define PXA_RUN     200
#define PXA_FFRUN   200
#define PYA_GRAVITY 2750
#define PYA_SINK    1000


// Enum

enum JJ2PlayerReaction {

	JJ2PR_NONE, JJ2PR_HURT, JJ2PR_KILLED, JJ2PR_INVINCIBLE

};

enum JJ2Shield {

	 JJ2S_NONE = 0, JJ2S_FLAME = 1, JJ2S_BUBBLE = 2, JJ2S_PLASMA = 3, JJ2S_LASER = 4

};


// Classes

class Anim;

class JJ2LevelPlayer : public Movable {

	private:
		bool              bird; // Placeholder for eventual JJ2Bird object
		SDL_Color         palette[256];
		char              anims[PANIMS];
		int               energy;
		JJ2Shield         shield;
		int               floating; /* 0 = normal, 1 = helicopter ears, 2 = boarding */
		bool              facing;
		unsigned char     animType;
		int               lookTime; /* Negative if looking up, positive if looking down, 0 if neither */
		JJ2PlayerReaction reaction;
		unsigned int      reactionTime;
		unsigned int      fireTime;
		fixed             jumpHeight;
		fixed             jumpY;
		unsigned int      fastFeetTime;
		unsigned char     warpX, warpY;
		unsigned int      warpTime;
		int               gems[4];

	public:
		Player* player;

		JJ2LevelPlayer  (Player* parent, char* newAnims, unsigned char startX, unsigned char startY, bool hasBird);
		~JJ2LevelPlayer ();

		void              reset       (unsigned char startX, unsigned char startY);

		void              addGem      (int colour);
		unsigned char     getAnim     ();
		int               getEnemies  ();
		int               getEnergy   ();
		bool              getFacing   ();
		int               getGems     (int colour);
		bool              hasBird     ();
		bool              hit         (Player* source, unsigned int ticks);
		void              kill        (Player* source, unsigned int ticks);
		bool              overlap     (fixed left, fixed top, fixed width, fixed height);
		JJ2PlayerReaction reacted     (unsigned int ticks);
		void              setPosition (fixed newX, fixed newY);
		void              setSpeed    (fixed newDx, fixed newDy);
		bool              takeEvent   (unsigned char gridX, unsigned char gridY, unsigned int ticks);
		bool              touchEvent  (unsigned char gridX, unsigned char gridY, unsigned int ticks, int msps);

		void              send        (unsigned char* buffer);
		void              receive     (unsigned char* buffer);

		void              control     (unsigned int ticks, int msps);
		void              move        (unsigned int ticks, int msps);
		void              view        (unsigned int ticks, int mspf);
		void              draw        (unsigned int ticks, int change);

};

#endif

