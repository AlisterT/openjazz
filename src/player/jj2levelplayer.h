
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

#include "jj2level/jj2level.h"
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
#define PA_WOOZYSHAKE   0
#define PA_BOARD        1
#define PA_BOARDSW      2
#define PA_STOMP        3
#define PA_DEAD         4
#define PA_DIE          5
#define PA_CROUCH1      6
#define PA_CROUCHED     7
#define PA_CROUCHSHOOT  8
#define PA_CROUCH2      9
#define PA_EXIT1        10
#define PA_VINE         11
#define PA_EXIT2        12
#define PA_FALL         13
#define PA_STOMPING     14
#define PA_LAND         15
#define PA_STANDSHOOT   16
#define PA_STANDSHOOTUP 17
#define PA_WHIP1        18
#define PA_UNFROG       19
#define PA_MOUNT        20
#define PA_HOOKWHIP     21
#define PA_HOOKDIAG     22
#define PA_HOOKSHOOTUP  23
#define PA_HOOK1        24
#define PA_HOOK2        25
#define PA_HOOKWHIPUP   26
#define PA_HOOKSHOOT    27
#define PA_HELI         28
#define PA_HELIWHIP     29
#define PA_HELISHOOT    30
#define PA_HPOLE        31
#define PA_HURT1        32
#define PA_WAIT1        33
#define PA_WAIT2        34
#define PA_WAIT3        35
#define PA_WAIT4        36
#define PA_WAIT5        37
#define PA_FALLWHIP     38
#define PA_FALLSHOOT    39
#define PA_FLOAT1       40
#define PA_FLOAT2       41
#define PA_UP1          42
#define PA_EDGE         43
#define PA_CARRY        44
#define PA_UNLOAD       45
#define PA_LOAD         46
#define PA_LOOKUP1      47
#define PA_WALK45       48
#define PA_WALK90       49
#define PA_WALK135      50
#define PA_WALK180      51
#define PA_WALK225      52
#define PA_WALK270      53
#define PA_WALK315      54
#define PA_WOOZYWALK    55
#define PA_PUSH         56
#define PA_WHIP2        57
#define PA_EXIT3        58
#define PA_SPEED1       59
#define PA_SPEED2       60
#define PA_FALLMOVE     61
#define PA_MYSTERY1     62
#define PA_JUMP2        63
#define PA_FALLMOVEWHIP 64
#define PA_MYSTERY2     65
#define PA_JUMPSHOOTUP  66
#define PA_BALL         67
#define PA_WALKSHOOT    68
#define PA_WALKDIAG     69
#define PA_RUN          70
#define PA_SPEEDRUN     71
#define PA_STOP1        72
#define PA_MYSTERY3     73
#define PA_STOP2        74
#define PA_UP2          75
#define PA_STAND        76
#define PA_POWER        77
#define PA_POWEREND     78
#define PA_POWERSTART   79
#define PA_WOOZYSTAND   80
#define PA_SWIMDOWN     81
#define PA_SWIM         82
#define PA_SWIMDIAGDOWN 83
#define PA_SWIMDIAGUP   84
#define PA_SWIMUP       85
#define PA_VINESDIAG    86
#define PA_WARPOUT      87
#define PA_WARPFALLIN   88
#define PA_WARPFALL     89
#define PA_WARPFALLOUT  90
#define PA_WARPIN       91
#define PA_VPOLE        92
#define PA_CROUCH3      93
#define PA_CROUCH4      94
#define PA_FALLSTRANGE1 95
#define PA_HURT2        96
#define PA_WAIT6        97
#define PA_FALLSTRANGE2 98
#define PA_CROUCH5      99
#define PA_LOOKUP2      100
#define PA_WALK2        101
#define PA_WORRY        102
#define PA_LOOKUP3      103

#define JJ2PANIMS       104 /* Number of player animations. */

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
#define PXS_LIMIT ITOF(500)
#define PYS_LIMIT ITOF(500)

#define PXS_WALK  ITOF(300)
#define PXS_RUN   ITOF(325)
#define PXS_FFRUN ITOF(500)
#define PYS_FALL  ITOF(350)
#define PYS_SINK  ITOF(150)
#define PYS_JUMP  ITOF(-350)

#define PXS_POLE   ITOF(500)
#define PYS_POLE   ITOF(500)
#define PYS_SPRING ITOF(-500)

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
class JJ2Event;

class JJ2LevelPlayer : public Movable {

	private:
		bool              bird; // Placeholder for eventual JJ2Bird object
		Anim*             anims;
		Anim*             flippedAnims;
		JJ2Modifier*      mod;
		SDL_Color         palette[256];
		int               energy;
		JJ2Shield         shield;
		int               floating; /* 0 = normal, 1 = helicopter ears, 2 = boarding */
		bool              facing;
		unsigned char     animType;
		PlayerEvent       event;
		int               lookTime; /* Negative if looking up, positive if looking down, 0 if neither */
		JJ2PlayerReaction reaction;
		unsigned int      reactionTime;
		unsigned int      fireTime;
		fixed             jumpHeight;
		fixed             throwX, throwY;
		unsigned int      fastFeetTime;
		unsigned int      stopTime;
		int               gems[4];
		int               coins;

		void              centreX ();
		void              centreY ();
		void              modify  (JJ2Modifier* nextMod, unsigned int ticks);

	public:
		Player* player;

		JJ2LevelPlayer  (Player* parent, Anim** newAnims, unsigned char startX, unsigned char startY, bool hasBird);
		~JJ2LevelPlayer ();

		void              reset       (unsigned char startX, unsigned char startY);

		void              addGem      (int colour);
		Anim*             getAnim     ();
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
		bool              touchEvent  (JJ2Event* touched, unsigned int ticks, int msps);

		void              send        (unsigned char* buffer);
		void              receive     (unsigned char* buffer);

		void              control     (unsigned int ticks, int msps);
		void              move        (unsigned int ticks, int msps);
		void              view        (unsigned int ticks, int mspf);
		void              draw        (unsigned int ticks, int change);

};

#endif

