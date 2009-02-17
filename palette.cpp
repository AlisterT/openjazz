
/*
 *
 * palette.cpp
 *
 * Created on the 4th of February 2009 from parts of main.cpp and util.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "OpenJazz.h"
#include <string.h>


PaletteEffect::PaletteEffect (unsigned char newType, unsigned char newFirst,
	unsigned char newAmount, fixed newSpeed, PaletteEffect * nextPE) {

	next = nextPE;
	type = newType;
	first = newFirst;
	amount = newAmount;
	speed = newSpeed;
	position = 0;

	return;

}


PaletteEffect::~PaletteEffect () {

	if (next) delete next;

	return;

}


unsigned char PaletteEffect::getType () {

	return type;

}


void PaletteEffect::setPosition (fixed newPosition) {

	position = newPosition;

	return;

}


void PaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int count, x, y;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);

	switch (type) {

		case PE_FADE:

			if (position) {

				for (count = first; count < first + amount; count++) {

					shownPalette[count].r = (currentPalette[count].r *
						position) >> 10;
					shownPalette[count].g = (currentPalette[count].g *
						position) >> 10;
					shownPalette[count].b = (currentPalette[count].b *
						position) >> 10;

				}

				position -= (mspf << 10) / speed;

			} else {

				memset(shownPalette + first, 0, sizeof(SDL_Color) * amount);

			}

			if (direct)
				SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
					amount);

			break;

		case PE_1D:

			for (count = 0; count < amount; count++) {

				memcpy(shownPalette + first + count,
					currentPalette + first + ((count + (amount - 1 -
					((((position >> 10) * speed) >> 10) % amount))) % amount),
					sizeof(SDL_Color));

			}

			if (direct)
				SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
					amount);

			break;

		case PE_ROTATE:

			for (count = 0; count < amount; count++) {

				memcpy(shownPalette + first + count,
					currentPalette + first +
					((count + (position >> 10)) % amount), sizeof(SDL_Color));

			}

			position -= (mspf * speed) >> 10;
			while (position < 0) position += amount << 10;

			if (direct)
				SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
					amount);

			break;

		case PE_SKY:

			if (screenW > 320) y = ((screenH - 1) / 100) + 1;
			else y = ((screenH - 34) / 100) + 1;

			count = (((position * speed) / y) >> 20) % 255;

			if (direct) {

				if (count > 255 - amount) {

					SDL_SetPalette(screen, SDL_PHYSPAL,
						levelInst->skyPalette + count, first, 255 - count);
					SDL_SetPalette(screen, SDL_PHYSPAL, levelInst->skyPalette,
						first + (255 - count),
						count + amount - 255);

				} else {

					SDL_SetPalette(screen, SDL_PHYSPAL,
						levelInst->skyPalette + count, first, amount);

				}

			} else {

				if (count > 255 - amount) {

					memcpy(shownPalette + first, levelInst->skyPalette + count,
						sizeof(SDL_Color) * (255 - count));
					memcpy(shownPalette + first + (255 - count),
						levelInst->skyPalette, sizeof(SDL_Color) *
						(count + amount - 255));

				} else {

					memcpy(shownPalette + first, levelInst->skyPalette + count,
						sizeof(SDL_Color) * amount);

				}

			}

			break;

		case PE_2D:

			x = (((256 * 32) - (position & 65535)) * speed) >> 10;
			y = (((64 * 32) - (position >> 16)) * speed) >> 10;

			for (count = 0; count < amount >> 3; count++) {

				int j;

				for (j = 0; j < 8; j++) {

					memcpy(shownPalette + first + (count << 3) + j,
						currentPalette + first + (((count + y) % 8) << 3) +
						((j + x) % 8), sizeof(SDL_Color));

				}

			}

			if (direct)
				SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
					amount);

			break;

		case PE_WATER:

			// Underwater darkness palette effect

			if (position <= 0) break;

			if (position < speed) {

				for (count = first; count < first + amount; count++) {

					shownPalette[count].r = (currentPalette[count].r *
						(1023 - ((position << 10) / speed))) >> 10;
					shownPalette[count].g = (currentPalette[count].g *
						(1023 - ((position << 10) / speed))) >> 10;
					shownPalette[count].b = (currentPalette[count].b *
						(1023 - ((position << 10) / speed))) >> 10;
				}

			} else memset(shownPalette + first, 0, sizeof(SDL_Color) * amount);

			if (direct)
				SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
					amount);

			break;

	}

	return;

}


void usePalette (SDL_Color *palette) {

	// Make palette changes invisible until the next draw. Hopefully.
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	SDL_Flip(screen);

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
	currentPalette = palette;

	return;

}


void scalePalette (SDL_Surface *surface, fixed scale, signed int offset) {

	SDL_Color palette[256];
	int count;

	for (count = 0; count < 256; count++) {

		palette[count].r = ((count * scale) >> 10) + offset;
		palette[count].g = ((count * scale) >> 10) + offset;
		palette[count].b = ((count * scale) >> 10) + offset;

	}

	SDL_SetPalette(surface, SDL_LOGPAL, palette, 0, 256);

	return;

}


void restorePalette (SDL_Surface *surface) {

	SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

	return;

}



