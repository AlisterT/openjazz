
/**
 *
 * @file bonusplayer.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 24th June 2010: Created bonusplayer.h from parts of player.h
 *
 * @section Licence
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

#ifndef _BONUSPLAYER_H
#define _BONUSPLAYER_H


#include "player.h"

#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Bonus level animations
#define PA_WALK  0
#define PA_LEFT  1
#define PA_RIGHT 2
#define PA_USER  3
#define PA_JUMP  4
#define PA_CRASH 5
#define PA_OTHER 6

#define BPANIMS  7

// Player speeds
#define PRS_REVERSE ITOF(-32)
#define PRS_RUN     ITOF(64)
#define PRS_ROLL    ITOF(96)

// Player accelerations
#define PRA_REVERSE 450
#define PRA_STOP    500
#define PRA_RUN     100


// Classes

class Anim;
class Bonus;

/// JJ1 bonus level player
class BonusPlayer {

	private:
		SDL_Color     palette[256]; ///< Palette (for custom colours)
		Anim*         anims[BPANIMS]; ///< Animations
		fixed         x; ///< X-coordinate
		fixed         y; ///< Y-coordinate
		fixed         direction; ///< Direction
		fixed         dr; ///< Forward speed
		unsigned char animType; ///< Current animation
		int           gems; ///< Number of gems collected

	public:
		Player* player; ///< Corresponding game player

		BonusPlayer  (Player* parent, Anim** newAnims, unsigned char startX, unsigned char startY);
		~BonusPlayer ();

		void          addGem       ();
		fixed         getDirection ();
		int           getGems      ();
		fixed         getX         ();
		fixed         getY         ();

		void          step         (unsigned int ticks, int msps, Bonus* bonus);
		void          draw         (unsigned int ticks);

};

#endif

