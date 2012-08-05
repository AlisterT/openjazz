
/**
 *
 * @file levelplayer.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 5th August 2012: Created levelplayer.h from parts of jj1levelplayer.h,
 *                  jj1bonuslevelplayer.h and jj2levelplayer.h
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _LEVELPLAYER_H
#define _LEVELPLAYER_H


#include "level/movable.h"

#include <SDL/SDL.h>


// Classes

class Player;

/// Level player
class LevelPlayer : public Movable {

	protected:
		SDL_Color palette[256]; ///< Palette (for custom colours)

	public:
		Player* player; ///< Corresponding game player

		virtual ~LevelPlayer ();

		virtual void reset   (unsigned char startX, unsigned char startY) = 0;

		virtual int  countBirds () = 0;

		virtual void send    (unsigned char* buffer) = 0;
		virtual void receive (unsigned char* buffer) = 0;

};

#endif

