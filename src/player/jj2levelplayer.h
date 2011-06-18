
/**
 *
 * @file jj2levelplayer.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 29th June 2010: Created jj2levelplayer.h from parts of levelplayer.h
 *
 * @section Licence
 * Copyright (c) 2005-2011 Alister Thomson
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
#define JJ2PA_WOOZYSHAKE   0
#define JJ2PA_BOARD        1
#define JJ2PA_BOARDSW      2
#define JJ2PA_STOMP        3
#define JJ2PA_DEAD         4
#define JJ2PA_DIE          5
#define JJ2PA_CROUCH1      6
#define JJ2PA_CROUCHED     7
#define JJ2PA_CROUCHSHOOT  8
#define JJ2PA_CROUCH2      9
#define JJ2PA_EXIT1        10
#define JJ2PA_VINE         11
#define JJ2PA_EXIT2        12
#define JJ2PA_FALL         13
#define JJ2PA_STOMPING     14
#define JJ2PA_LAND         15
#define JJ2PA_STANDSHOOT   16
#define JJ2PA_STANDSHOOTUP 17
#define JJ2PA_WHIP1        18
#define JJ2PA_UNFROG       19
#define JJ2PA_MOUNT        20
#define JJ2PA_HOOKWHIP     21
#define JJ2PA_HOOKDIAG     22
#define JJ2PA_HOOKSHOOTUP  23
#define JJ2PA_HOOK1        24
#define JJ2PA_HOOK2        25
#define JJ2PA_HOOKWHIPUP   26
#define JJ2PA_HOOKSHOOT    27
#define JJ2PA_HELI         28
#define JJ2PA_HELIWHIP     29
#define JJ2PA_HELISHOOT    30
#define JJ2PA_HPOLE        31
#define JJ2PA_HURT1        32
#define JJ2PA_WAIT1        33
#define JJ2PA_WAIT2        34
#define JJ2PA_WAIT3        35
#define JJ2PA_WAIT4        36
#define JJ2PA_WAIT5        37
#define JJ2PA_FALLWHIP     38
#define JJ2PA_FALLSHOOT    39
#define JJ2PA_FLOAT1       40
#define JJ2PA_FLOAT2       41
#define JJ2PA_UP1          42
#define JJ2PA_EDGE         43
#define JJ2PA_CARRY        44
#define JJ2PA_UNLOAD       45
#define JJ2PA_LOAD         46
#define JJ2PA_LOOKUP1      47
#define JJ2PA_WALK45       48
#define JJ2PA_WALK90       49
#define JJ2PA_WALK135      50
#define JJ2PA_WALK180      51
#define JJ2PA_WALK225      52
#define JJ2PA_WALK270      53
#define JJ2PA_WALK315      54
#define JJ2PA_WOOZYWALK    55
#define JJ2PA_PUSH         56
#define JJ2PA_WHIP2        57
#define JJ2PA_EXIT3        58
#define JJ2PA_SPEED1       59
#define JJ2PA_SPEED2       60
#define JJ2PA_FALLMOVE     61
#define JJ2PA_MYSTERY1     62
#define JJ2PA_JUMP2        63
#define JJ2PA_FALLMOVEWHIP 64
#define JJ2PA_MYSTERY2     65
#define JJ2PA_JUMPSHOOTUP  66
#define JJ2PA_BALL         67
#define JJ2PA_WALKSHOOT    68
#define JJ2PA_WALKDIAG     69
#define JJ2PA_RUN          70
#define JJ2PA_SPEEDRUN     71
#define JJ2PA_STOP1        72
#define JJ2PA_MYSTERY3     73
#define JJ2PA_STOP2        74
#define JJ2PA_UP2          75
#define JJ2PA_STAND        76
#define JJ2PA_POWER        77
#define JJ2PA_POWEREND     78
#define JJ2PA_POWERSTART   79
#define JJ2PA_WOOZYSTAND   80
#define JJ2PA_SWIMDOWN     81
#define JJ2PA_SWIM         82
#define JJ2PA_SWIMDIAGDOWN 83
#define JJ2PA_SWIMDIAGUP   84
#define JJ2PA_SWIMUP       85
#define JJ2PA_VINESDIAG    86
#define JJ2PA_WARPOUT      87
#define JJ2PA_WARPFALLIN   88
#define JJ2PA_WARPFALL     89
#define JJ2PA_WARPFALLOUT  90
#define JJ2PA_WARPIN       91
#define JJ2PA_VPOLE        92
#define JJ2PA_CROUCH3      93
#define JJ2PA_CROUCH4      94
#define JJ2PA_FALLSTRANGE1 95
#define JJ2PA_HURT2        96
#define JJ2PA_WAIT6        97
#define JJ2PA_FALLSTRANGE2 98
#define JJ2PA_CROUCH5      99
#define JJ2PA_LOOKUP2      100
#define JJ2PA_WALK2        101
#define JJ2PA_WORRY        102
#define JJ2PA_LOOKUP3      103

#define JJ2PANIMS       104 /* Number of player animations. */

// Player reaction times
#define JJ2PRT_HURT       1000
#define JJ2PRT_HURTANIM   200
#define JJ2PRT_KILLED     2000
#define JJ2PRT_INVINCIBLE 10000

// Other time periods
#define T_FASTFEET 25000
#define T_WARP     1000

// Player offsets
#define JJ2PXO_MID F16
#define JJ2PXO_L    (JJ2PXO_MID - F10)
#define JJ2PXO_ML   (JJ2PXO_MID - F4)
#define JJ2PXO_MR   (JJ2PXO_MID + F4)
#define JJ2PXO_R    (JJ2PXO_MID + F10)
#define JJ2PYO_TOP  (-F20)
#define JJ2PYO_MID  (-F10)
#define JJ2PYO_JUMP ITOF(92)

// Player speeds
#define JJ2PXS_LIMIT ITOF(500)
#define JJ2PYS_LIMIT ITOF(500)

#define JJ2PXS_WALK  ITOF(300)
#define JJ2PXS_RUN   ITOF(325)
#define JJ2PXS_FFRUN ITOF(500)
#define JJ2PYS_FALL  ITOF(350)
#define JJ2PYS_SINK  ITOF(150)
#define JJ2PYS_JUMP  ITOF(-350)

#define JJ2PXS_POLE   ITOF(500)
#define JJ2PYS_POLE   ITOF(500)
#define JJ2PYS_SPRING ITOF(-500)

// Player accelerations
#define JJ2PXA_REVERSE 900
#define JJ2PXA_STOP    1000
#define JJ2PXA_WALK    500
#define JJ2PXA_RUN     200
#define JJ2PXA_FFRUN   200
#define JJ2PYA_GRAVITY 2750
#define JJ2PYA_SINK    1000


// Enum

/// JJ2 player reaction type
enum JJ2PlayerReaction {

	JJ2PR_NONE, JJ2PR_HURT, JJ2PR_KILLED, JJ2PR_INVINCIBLE

};

/// JJ2 shield type
enum JJ2Shield {

	 JJ2S_NONE = 0, JJ2S_FLAME = 1, JJ2S_BUBBLE = 2, JJ2S_PLASMA = 3, JJ2S_LASER = 4

};


// Classes

class Anim;
class JJ2Event;

/// JJ2 level player
class JJ2LevelPlayer : public Movable {

	private:
		int               birds; ///< Placeholder for eventual JJ2Bird objects
		Anim*             anims; ///< Animations
		Anim*             flippedAnims; ///< Animations (flipped)
		JJ2Modifier*      mod; ///< Modifier currently affecting player
		SDL_Color         palette[256]; ///< Palette (for custom colours)
		int               energy; ///< 0 = dead, 3 or 5 = maximum
		JJ2Shield         shield; ///< Current shield
		int               floating; ///< 0 = normal, 1 = helicopter ears, 2 = boarding
		bool              facing; ///< false = left, true = right
		unsigned char     animType; ///< Current animation
		PlayerEvent       event; ///< Event type
		int               lookTime; ///< Negative if looking up, positive if looking down, 0 if neither
		JJ2PlayerReaction reaction; ///< Reaction type
		unsigned int      reactionTime; ///< Time reaction will end
		unsigned int      fireTime; ///< The next time the player can fire
		fixed             jumpHeight; ///< The height the player can reach when jumping
		fixed             throwX; ///< Having been thrown, the x-coordinate the player can reach
		fixed             throwY; ///< Having been thrown, the y-coordinate the player can reach
		unsigned int      fastFeetTime;
		unsigned int      stopTime;
		int               gems[4]; ///< Gems collected
		int               coins; ///< Value of coins collected

		void              centreX ();
		void              centreY ();
		void              modify  (JJ2Modifier* nextMod, unsigned int ticks);

	public:
		Player* player; ///< Corresponding game player

		JJ2LevelPlayer  (Player* parent, Anim** newAnims, unsigned char startX, unsigned char startY, int flockSize);
		~JJ2LevelPlayer ();

		void              reset       (unsigned char startX, unsigned char startY);

		void              addGem      (int colour);
		int               countBirds  ();
		Anim*             getAnim     ();
		int               getEnemies  ();
		int               getEnergy   ();
		bool              getFacing   ();
		int               getGems     (int colour);
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

