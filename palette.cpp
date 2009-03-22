
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


PaletteEffect::PaletteEffect (unsigned char newFirst, int newAmount,
	fixed newSpeed, PaletteEffect * nextPE) {

	next = nextPE;
	first = newFirst;
	amount = newAmount;
	speed = newSpeed;

	return;

}


PaletteEffect::~PaletteEffect () {

	if (next) delete next;

	return;

}


void PaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);

	return;

}


WhiteInPaletteEffect::WhiteInPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	whiteness = F1 + FH;

	return;

}


void WhiteInPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	if (whiteness > F1) {

		memset(shownPalette + first, 255, sizeof(SDL_Color) * amount);

		whiteness -= (mspf << 10) / speed;

	} else if (whiteness > 0) {

		for (count = first; count < first + amount; count++) {

			shownPalette[count].r = 255 -
				(((255 - shownPalette[count].r) * (F1 - whiteness)) >> 10);
			shownPalette[count].g = 255 -
				(((255 - shownPalette[count].g) * (F1 - whiteness)) >> 10);
			shownPalette[count].b = 255 -
				(((255 - shownPalette[count].b) * (F1 - whiteness)) >> 10);

		}

		whiteness -= (mspf << 10) / speed;

	}

	if (direct)
		SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
			amount);

	return;

}


FadeInPaletteEffect::FadeInPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	blackness = F1 + FH;

	return;

}


void FadeInPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	if (blackness > F1) {

		memset(shownPalette + first, 0, sizeof(SDL_Color) * amount);

		blackness -= (mspf << 10) / speed;

	} else if (blackness > 0) {

		for (count = first; count < first + amount; count++) {

			shownPalette[count].r =
				(shownPalette[count].r * (F1 - blackness)) >> 10;
			shownPalette[count].g =
				(shownPalette[count].g * (F1 - blackness)) >> 10;
			shownPalette[count].b =
				(shownPalette[count].b * (F1 - blackness)) >> 10;

		}

		blackness -= (mspf << 10) / speed;

	}

	if (direct)
		SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
			amount);

	return;

}


WhiteOutPaletteEffect::WhiteOutPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	whiteness = -F8;

	return;

}


void WhiteOutPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	if (whiteness > F1) {

		memset(shownPalette + first, 255, sizeof(SDL_Color) * amount);

	} else if (whiteness > 0) {

		for (count = first; count < first + amount; count++) {

			shownPalette[count].r = 255 -
				(((255 - shownPalette[count].r) * (F1 - whiteness)) >> 10);
			shownPalette[count].g = 255 -
				(((255 - shownPalette[count].g) * (F1 - whiteness)) >> 10);
			shownPalette[count].b = 255 -
				(((255 - shownPalette[count].b) * (F1 - whiteness)) >> 10);

		}

		whiteness += (mspf << 10) / speed;

	} else whiteness += (mspf << 10) / speed;

	if (direct)
		SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
			amount);

	return;

}


FadeOutPaletteEffect::FadeOutPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	blackness = -(F2 + F1);

	return;

}


void FadeOutPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	if (blackness > F1) {

		memset(shownPalette + first, 0, sizeof(SDL_Color) * amount);

	} else if (blackness > 0) {

		for (count = first; count < first + amount; count++) {

			shownPalette[count].r =
				(shownPalette[count].r * (F1 - blackness)) >> 10;
			shownPalette[count].g =
				(shownPalette[count].g * (F1 - blackness)) >> 10;
			shownPalette[count].b =
				(shownPalette[count].b * (F1 - blackness)) >> 10;

		}

		blackness += (mspf << 10) / speed;

	} else blackness += (mspf << 10) / speed;

	if (direct)
		SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
			amount);

	return;

}


RotatePaletteEffect::RotatePaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	position = 0;

	return;

}


void RotatePaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


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

	return;

}


SkyPaletteEffect::SkyPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, SDL_Color *newSkyPalette,
	PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	skyPalette = newSkyPalette;

	return;

}


void SkyPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int position, count, y;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	position = viewY + (viewH << 9) - F4;

	if (screenW > 320) y = ((screenH - 1) / 100) + 1;
	else y = ((screenH - 34) / 100) + 1;

	count = (((position * speed) / y) >> 20) % 255;

	if (direct) {

		if (count > 255 - amount) {

			SDL_SetPalette(screen, SDL_PHYSPAL, skyPalette + count, first,
				255 - count);
			SDL_SetPalette(screen, SDL_PHYSPAL, skyPalette,
				first + (255 - count), count + amount - 255);

		} else {

			SDL_SetPalette(screen, SDL_PHYSPAL, skyPalette + count,
				first, amount);

		}

	} else {

		if (count > 255 - amount) {

			memcpy(shownPalette + first, skyPalette + count,
				sizeof(SDL_Color) * (255 - count));
			memcpy(shownPalette + first + (255 - count), skyPalette,
				sizeof(SDL_Color) * (count + amount - 255));

		} else {

			memcpy(shownPalette + first, skyPalette + count,
				sizeof(SDL_Color) * amount);

		}

	}

	return;

}


P2DPaletteEffect::P2DPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	return;

}


void P2DPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int count, x, y, j;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	x = (((256 * 32) - (viewX >> 10)) * speed) >> 10;
	y = (((64 * 32) - (viewY >> 10)) * speed) >> 10;

	for (count = 0; count < amount >> 3; count++) {

		for (j = 0; j < 8; j++) {

			memcpy(shownPalette + first + (count << 3) + j,
				currentPalette + first + (((count + y) % 8) << 3) +
				((j + x) % 8), sizeof(SDL_Color));

		}

	}

	if (direct)
		SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
			amount);

	return;

}


P1DPaletteEffect::P1DPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	return;

}


void P1DPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int position, count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	position = viewX + viewY;

	for (count = 0; count < amount; count++) {

		memcpy(shownPalette + first + count,
			currentPalette + first + ((count + (amount - 1 -
			((((position >> 10) * speed) >> 10) % amount))) % amount),
			sizeof(SDL_Color));

	}

	if (direct)
		SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette + first, first,
			amount);

	return;

}


WaterPaletteEffect::WaterPaletteEffect (unsigned char newFirst,
	int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (newFirst, newAmount, newSpeed, nextPE) {

	return;

}


void WaterPaletteEffect::apply (SDL_Color *shownPalette, bool direct) {

	int position, count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct);


	position = viewY - level->getWaterLevel(0);

	if (position <= 0) return;

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

	return;

}


void usePalette (SDL_Color *palette) {

	// Make palette changes invisible until the next draw. Hopefully.
	clearScreen(SDL_MapRGB(screen->format, 0, 0, 0));
	SDL_Flip(screen);

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
	currentPalette = palette;

	return;

}


void restorePalette (SDL_Surface *surface) {

	SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

	return;

}



