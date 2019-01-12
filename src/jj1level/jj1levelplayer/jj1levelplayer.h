
/**
 *
 * @file jj1levelplayer.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created player.h from parts of OpenJazz.h
 * - 24th June 2010: Created levelplayer.h from parts of player.h
 * - 29th June 2010: Created jj2levelplayer.h from parts of levelplayer.h
 * - 1st August 2012: Renamed levelplayer.h to jj1levelplayer.h
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


#ifndef _JJ1LEVELPLAYER_H
#define _JJ1LEVELPLAYER_H


#include "../jj1level.h"

#include "level/levelplayer.h"
#include "player/player.h"
#include "OpenJazz.h"


// Constants

// Colour offsets
#define PCO_GREY   0
#define PCO_SGREEN 16
#define PCO_BLUE   23
#define PCO_RED    32
#define PCO_LGREEN 48
#define PCO_LEVEL1 64
#define PCO_YELLOW 75
#define PCO_LEVEL2 80
#define PCO_ORANGE 88
#define PCO_LEVEL3 96
#define PCO_LEVEL4 104
#define PCO_SANIM  112
#define PCO_LANIM  116
#define PCO_LEVEL5 124

// Player reaction times
#define PRT_HURT       1000
#define PRT_HURTANIM   200
#define PRT_KILLED     2000
#define PRT_INVINCIBLE 10000
#define PRT_SHIELDED   100

// Other time periods
#define T_FASTFEET 25000
#define T_WARP     1000
#define T_FIRING   150

// Player offsets
#define PXO_MID F16
#define PXO_L    (PXO_MID - F12)
#define PXO_ML   (PXO_MID - F8)
#define PXO_MR   (PXO_MID + F8)
#define PXO_R    (PXO_MID + F12)
#define PYO_TOP  (-F20)
#define PYO_MID  (-F10)
#define PYO_JUMP ITOF(84)

// Player speeds
#define PXS_WALK  ITOF(150)
#define PXS_RUN   ITOF(325)
#define PYS_FALL  ITOF(350)
#define PYS_SINK  ITOF(150)
#define PYS_JUMP  -ITOF(350)

// Player accelerations
#define PXA_REVERSE ITOF(28)
#define PXA_STOP    ITOF(16)
#define PXA_WALK    ITOF(16)
#define PXA_RUN     ITOF(16)
#define PYA_GRAVITY ITOF(36)
#define PYA_SINK    ITOF(16)


// Enums

/// JJ1 player event types
enum JJ1PlayerEvent {

	JJ1PE_NONE, ///< No event
	JJ1PE_SPRING, ///< Spring
	JJ1PE_FLOAT, ///< Float up
	JJ1PE_FLOATH, ///< Float horizontally
	JJ1PE_REPELH, ///< Repel horizontally only
	JJ1PE_REPELUP, ///< Repel upwards
	JJ1PE_REPELDOWN, ///< Repel downwards
	JJ1PE_PLATFORM ///< Moving platform

};

/// JJ1 player reaction type
enum JJ1PlayerReaction {

	PR_NONE, ///< Not reacting
	PR_HURT, ///< Hurt
	PR_KILLED, ///< Killed
	PR_INVINCIBLE, ///< Invincibility
	PR_SHIELDED ///< Hit to shield

};


// Classes

class Anim;
class JJ1Bird;

/// JJ1 level player
class JJ1LevelPlayer : public LevelPlayer {

	private:
		JJ1Bird*          birds; ///< Bird companion(s)
		Anim*             anims[JJ1PANIMS]; ///< Animations
		int               energy; ///< 0 = dead, 4 = maximum
		int               shield; ///< 0 = none, 1 = yellow, 2 = 1 orange, 3 = 2 orange, 4 = 3 orange, 5 = 4 orange
		bool              flying; ///< false = normal, true = boarding/bird/etc.
		bool              facing; ///< false = left, true = right
		fixed             udx; ///< Unobstructed horizontal speed
		unsigned char     animType; ///< Current animation
		unsigned char     eventX; ///< X-coordinate (in tiles) of an event (spring, platform, bridge)
		unsigned char     eventY; ///< Y-coordinate (in tiles) of an event (spring, platform, bridge)
		JJ1PlayerEvent    eventType; ///< Event type
		int               lookTime; ///< Negative if looking up, positive if looking down, 0 if neither
		JJ1PlayerReaction reaction; ///< Reaction type
		unsigned int      reactionTime; ///< Time the reaction will end
		unsigned int      fireTime; ///< The next time the player can fire
		unsigned int      fireAnimTime; ///< Time the firing animation will end
		fixed             jumpHeight; ///< The height the player can reach when jumping
		fixed             targetY; ///< Having been propelled, the y-coordinate the player could reach
		unsigned int      fastFeetTime; ///< Time fast feet will expire
		unsigned char     warpX; ///< X-coordinate (in tiles) player will warp to
		unsigned char     warpY; ///< Y-coordinate (in tiles) player will warp to
		unsigned int      warpTime; ///< Time the warp will happen
		int               enemies; ///< Number of enemies killed
		int               items; ///< Number of items collected
		bool              gem; ///< Bonus level gem collected

		bool checkMaskDown (fixed yOffset);
		bool checkMaskUp   (fixed yOffset);

		void ground ();

	public:
		JJ1LevelPlayer  (Player* parent, Anim** newAnims, unsigned char startX, unsigned char startY, int flockSize);
		~JJ1LevelPlayer ();

		void reset (int startX, int startY);

		void              addItem     ();
		void              clearEvent  (unsigned char gridX, unsigned char gridY);
		int               countBirds  ();
		Anim*             getAnim     ();
		int               getEnemies  ();
		int               getEnergy   ();
		bool              getFacing   ();
		int               getXSpeed   ();
		int               getItems    ();
		bool              hasGem      ();
		bool              hit         (Player* source, unsigned int ticks);
		void              kill        (Player* source, unsigned int ticks);
		bool              overlap     (fixed left, fixed top, fixed width, fixed height);
		JJ1PlayerReaction reacted     (unsigned int ticks);
		void              setPlatform (unsigned char gridX, unsigned char gridY, fixed shiftX, fixed newY);
		bool              takeEvent   (JJ1EventType* set, unsigned char gridX, unsigned char gridY, unsigned int ticks);
		bool              touchEvent  (JJ1EventType* set, unsigned char gridX, unsigned char gridY, unsigned int ticks);

		void           send        (unsigned char* buffer);
		void           receive     (unsigned char* buffer);

		void           changeAmmo  (int type, bool fallback = false);
		void           control     (unsigned int ticks);
		void           move        (unsigned int ticks);
		void           view        (unsigned int ticks, int mspf, int change);
		void           draw        (unsigned int ticks, int change);

};

#endif

