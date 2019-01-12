
/**
 *
 * @file jj2levelplayer.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created player.h from parts of OpenJazz.h
 * - 24th June 2010: Created levelplayer.h from parts of player.h
 * - 29th June 2010: Created jj2levelplayer.h from parts of levelplayer.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _JJ2LEVELPLAYER_H
#define _JJ2LEVELPLAYER_H


#include "../jj2level.h"

#include "level/levelplayer.h"
#include "player/player.h"
#include "OpenJazz.h"


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
#define JJ2PYS_JUMP  -ITOF(350)

#define JJ2PXS_POLE   ITOF(500)
#define JJ2PYS_POLE   ITOF(500)
#define JJ2PYS_SPRING -ITOF(500)

// Player accelerations
#define JJ2PXA_REVERSE 900
#define JJ2PXA_STOP    1000
#define JJ2PXA_WALK    500
#define JJ2PXA_RUN     200
#define JJ2PXA_FFRUN   200
#define JJ2PYA_GRAVITY 2750
#define JJ2PYA_SINK    1000


// Enum

/// JJ2 player event types
enum JJ2PlayerEvent {

	JJ2PE_NONE, ///< No event
	JJ2PE_SPRING, ///< Spring
	JJ2PE_FLOAT, ///< Float up
	JJ2PE_PLATFORM ///< Moving platform

};

/// JJ2 player reaction type
enum JJ2PlayerReaction {

	JJ2PR_NONE, ///< Not reacting
	JJ2PR_HURT, ///< Hurt
	JJ2PR_KILLED, ///< Killed
	JJ2PR_INVINCIBLE ///< Invincibility

};

/// JJ2 shield type
enum JJ2Shield {

	 JJ2S_NONE = 0, ///< No shield
	 JJ2S_FLAME = 1, ///< Flame shield
	 JJ2S_BUBBLE = 2, ///< Bubble shield
	 JJ2S_PLASMA = 3, ///< Plasma shield
	 JJ2S_LASER = 4 ///< Laser shield

};


// Classes

class Anim;
class JJ2Event;

/// JJ2 level player
class JJ2LevelPlayer : public LevelPlayer {

	private:
		int               birds; ///< Placeholder for eventual JJ2Bird objects
		Anim*             anims[JJ2PANIMS]; ///< Animations
		Anim*             flippedAnims[JJ2PANIMS]; ///< Animations (flipped)
		JJ2Modifier*      mod; ///< Modifier currently affecting player
		int               energy; ///< 0 = dead, 3 or 5 = maximum
		JJ2Shield         shield; ///< Current shield
		int               floating; ///< 0 = normal, 1 = helicopter ears, 2 = boarding
		bool              facing; ///< false = left, true = right
		unsigned char     animType; ///< Current animation
		JJ2PlayerEvent    event; ///< Event type
		int               lookTime; ///< Negative if looking up, positive if looking down, 0 if neither
		JJ2PlayerReaction reaction; ///< Reaction type
		unsigned int      reactionTime; ///< Time reaction will end
		unsigned int      fireTime; ///< The next time the player can fire
		fixed             jumpHeight; ///< The height the player can reach when jumping
		fixed             throwX; ///< Having been thrown, the x-coordinate the player can reach
		fixed             throwY; ///< Having been thrown, the y-coordinate the player can reach
		unsigned int      fastFeetTime; ///< Time fast feet will expire
		unsigned int      stopTime; ///< Time a modifier will cease stopping the player
		int               gems[4]; ///< Gems collected
		int               coins; ///< Value of coins collected

		bool checkMaskDown (fixed yOffset, bool drop);
		bool checkMaskUp   (fixed yOffset);

		void              centreX ();
		void              centreY ();
		void              ground  ();
		void              modify  (JJ2Modifier* nextMod, unsigned int ticks);

	public:
		JJ2LevelPlayer  (Player* parent, Anim** newAnims, Anim** newFlippedAnims, unsigned char startX, unsigned char startY, int flockSize);
		~JJ2LevelPlayer ();

		void              reset       (int startX, int startY);

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
		void              view        (unsigned int ticks, int mspf, int change);
		void              draw        (unsigned int ticks, int change);

};

#endif

