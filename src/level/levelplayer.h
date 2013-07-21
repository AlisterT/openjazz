
/**
 *
 * @file levelplayer.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 31st January 2006: Created player.h from parts of OpenJazz.h
 * 24th June 2010: Created bonusplayer.h and levelplayer.h from parts of
 *                 player.h
 * 29th June 2010: Created jj2levelplayer.h from parts of levelplayer.h
 * 1st August 2012: Renamed levelplayer.h to jj1levelplayer.h
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

		virtual void reset   (int startX, int startY) = 0;

		virtual int  countBirds () = 0;

		virtual void send    (unsigned char* buffer) = 0;
		virtual void receive (unsigned char* buffer) = 0;

};

#endif

