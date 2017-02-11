
/**
 *
 * @file sprite.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 31st January 2006: Created level.h from parts of OpenJazz.h
 * 19th March 2009: Created sprite.h from parts of level.h
 * 26th July 2009: Created anim.h from parts of sprite.h
 *
 * @section Licence
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _SPRITE_H
#define _SPRITE_H


#include "OpenJazz.h"

#include <SDL/SDL.h>


// Class

/// Sprite
class Sprite {

	private:
		SDL_Surface* pixels; ///< Sprite image
		short int    xOffset; ///< Horizontal offset
		short int    yOffset; ///< Vertical offset

	public:
		Sprite              ();
		~Sprite             ();

		void clearPixels    ();
		void setOffset      (short int x, short int y);
		void setPixels      (unsigned char* data, int width, int height, unsigned char key);
		int  getWidth       ();
		int  getHeight      ();
		int  getXOffset     ();
		int  getYOffset     ();
		void draw           (int x, int y, bool includeOffsets = true);
		void drawScaled     (int x, int y, fixed scale);
		void setPalette     (SDL_Color* palette, int start, int amount);
		void flashPalette   (int index);
		void restorePalette ();

};

#endif

