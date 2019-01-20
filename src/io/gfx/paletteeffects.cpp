
/**
 *
 * @file paletteeffects.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created main.c
 * - 22nd July 2008: Created util.c from parts of main.c
 * - 3rd February 2009: Renamed main.c to main.cpp and util.c to util.cpp
 * - 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * - 1st August 2009: Renamed palette.cpp to paletteeffects.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#include "paletteeffects.h"
#include "video.h"

#include "jj1level/jj1level.h"
#include "jj2level/jj2level.h"
#include "level/levelplayer.h"
#include "player/player.h"

#include <string.h>


/**
 * Create a new palette effect.
 *
 * @param nextPE The next palette effect
 */
PaletteEffect::PaletteEffect (PaletteEffect* nextPE) {

	next = nextPE;

	return;

}


/**
 * Delete the palette effect.
 */
PaletteEffect::~PaletteEffect () {

	if (next) delete next;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void PaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	return;

}


/**
 * Create a new white-in palette effect.
 *
 * @param newDuration The length of time the effect will last
 * @param nextPE The next palette effect
 */
WhiteInPaletteEffect::WhiteInPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	whiteness = F1 + FH;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void WhiteInPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	if (whiteness > F1) {

		memset(shownPalette, 255, sizeof(SDL_Color) * 256);

		if (!isStatic) whiteness -= ITOF(mspf) / duration;

	} else if (whiteness > 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = 255 -
				FTOI((255 - shownPalette[count].r) * (F1 - whiteness));
			shownPalette[count].g = 255 -
				FTOI((255 - shownPalette[count].g) * (F1 - whiteness));
			shownPalette[count].b = 255 -
				FTOI((255 - shownPalette[count].b) * (F1 - whiteness));

		}

		if (!isStatic) whiteness -= ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


/**
 * Create a new fade-in palette effect.
 *
 * @param newDuration The length of time the effect will last
 * @param nextPE The next palette effect
 */
FadeInPaletteEffect::FadeInPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	blackness = F1 + FH;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void FadeInPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);


	if (blackness > F1) {

		memset(shownPalette, 0, sizeof(SDL_Color) * 256);

		if (!isStatic) blackness -= ITOF(mspf) / duration;

	} else if (blackness > 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r =
				FTOI(shownPalette[count].r * (F1 - blackness));
			shownPalette[count].g =
				FTOI(shownPalette[count].g * (F1 - blackness));
			shownPalette[count].b =
				FTOI(shownPalette[count].b * (F1 - blackness));

		}

		if (!isStatic) blackness -= ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


/**
 * Create a new white-out palette effect.
 *
 * @param newDuration The length of time the effect will last
 * @param nextPE The next palette effect
 */
WhiteOutPaletteEffect::WhiteOutPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	whiteness = 0;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void WhiteOutPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);


	if (whiteness > F1) {

		memset(shownPalette, 255, sizeof(SDL_Color) * 256);

	} else {

		if (whiteness > 0) {

			for (count = 0; count < 256; count++) {

				shownPalette[count].r = 255 -
					FTOI((255 - shownPalette[count].r) * (F1 - whiteness));
				shownPalette[count].g = 255 -
					FTOI((255 - shownPalette[count].g) * (F1 - whiteness));
				shownPalette[count].b = 255 -
					FTOI((255 - shownPalette[count].b) * (F1 - whiteness));

			}
		}

		if (!isStatic) whiteness += ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


/**
 * Create a new fade-out palette effect.
 *
 * @param newDuration The length of time the effect will last
 * @param nextPE The next palette effect
 */
FadeOutPaletteEffect::FadeOutPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	blackness = -(F2 + F1);

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void FadeOutPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	if (blackness > F1) {

		memset(shownPalette, 0, sizeof(SDL_Color) * 256);

	} else {

		if (blackness > 0) {

			for (count = 0; count < 256; count++) {

				shownPalette[count].r =
					FTOI(shownPalette[count].r * (F1 - blackness));
				shownPalette[count].g =
					FTOI(shownPalette[count].g * (F1 - blackness));
				shownPalette[count].b =
					FTOI(shownPalette[count].b * (F1 - blackness));

			}

		}

		if (!isStatic) blackness += ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


/**
 * Create a new flash-to-colour palette effect.
 *
 * @param newRed The red component of the colour
 * @param newGreen The green component of the colour
 * @param newBlue The blue component of the colour
 * @param newDuration The length of time the effect will last
 * @param nextPE The next palette effect
 */
FlashPaletteEffect::FlashPaletteEffect
	(unsigned char newRed, unsigned char newGreen, unsigned char newBlue,
		int newDuration, PaletteEffect* nextPE) :
	PaletteEffect (nextPE) {

	duration = newDuration;
	progress = -F1;
	red = newRed;
	green = newGreen;
	blue = newBlue;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void FlashPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	if (progress < 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = FTOI((shownPalette[count].r * -progress) +
				(red * (progress + F1)));
			shownPalette[count].g = FTOI((shownPalette[count].g * -progress) +
				(green * (progress + F1)));
			shownPalette[count].b = FTOI((shownPalette[count].b * -progress) +
				(blue * (progress + F1)));

		}

		if (!isStatic) progress += ITOF(mspf) / duration;

	} else if (progress < F1) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = FTOI((shownPalette[count].r * progress) +
				(red * (F1 - progress)));
			shownPalette[count].g = FTOI((shownPalette[count].g * progress) +
				(green * (F1 - progress)));
			shownPalette[count].b = FTOI((shownPalette[count].b * progress) +
				(blue * (F1 - progress)));

		}

		if (!isStatic) progress += ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


/**
 * Create a new colour rotation palette effect.
 *
 * @param newFirst The first palette index to be affected
 * @param newAmount The number of palette indices to be affected
 * @param newSpeed The speed of he rotation
 * @param nextPE The next palette effect
 */
RotatePaletteEffect::RotatePaletteEffect
	(unsigned char newFirst, int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (nextPE) {

	first = newFirst;
	amount = newAmount;
	speed = newSpeed;
	position = 0;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void RotatePaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	SDL_Color* currentPalette;
	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	currentPalette = video.getPalette();

	for (count = 0; count < amount; count++) {

		memcpy(shownPalette + first + count,
			currentPalette + first +
			((count + FTOI(position)) % amount), sizeof(SDL_Color));

	}

	if (!isStatic) {

		position -= (mspf * speed) >> 10;
		while (position < 0) position += ITOF(amount);

	}

	if (direct) video.changePalette(shownPalette + first, first, amount);

	return;

}


/**
 * Create a new parallaxing sky background palette effect.
 *
 * @param newFirst The first palette index to be affected
 * @param newAmount The number of palette indices to be affected
 * @param newSpeed The relative speed of the background
 * @param newSkyPalette Palette containing the sky's colours
 * @param nextPE The next palette effect
 */
SkyPaletteEffect::SkyPaletteEffect
	(unsigned char newFirst, int newAmount, fixed newSpeed,
		SDL_Color* newSkyPalette, PaletteEffect* nextPE) :
	PaletteEffect (nextPE) {

	skyPalette = newSkyPalette;
	first = newFirst;
	amount = newAmount;
	speed = newSpeed;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void SkyPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	int position, count, y;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	position = viewY + ((canvasH - 33) << 9) - F4;
	y = ((canvasH - 34) / 100) + 1;
	count = (((position * speed) / y) >> 20) % 255;

	if (direct) {

		if (count > 255 - amount) {

			video.changePalette(skyPalette + count, first, 255 - count);
			video.changePalette(skyPalette, first + (255 - count), count + amount - 255);

		} else {

			video.changePalette(skyPalette + count, first, amount);

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


/**
 * Create a new 2D parallaxing background palette effect.
 *
 * @param newFirst The first palette index to be affected
 * @param newAmount The number of palette indices to be affected
 * @param newSpeed The relative speed of the background
 * @param nextPE The next palette effect
 */
P2DPaletteEffect::P2DPaletteEffect
	(unsigned char newFirst, int newAmount, fixed newSpeed, PaletteEffect* nextPE) :
	PaletteEffect (nextPE) {

	first = newFirst;
	amount = newAmount;
	speed = newSpeed;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void P2DPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	SDL_Color* currentPalette;
	int count, x, y, j;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	currentPalette = video.getPalette();
	x = FTOI(((256 * 32) - FTOI(viewX)) * speed);
	y = FTOI(((64 * 32) - FTOI(viewY)) * speed);

	for (count = 0; count < amount >> 3; count++) {

		for (j = 0; j < 8; j++) {

			memcpy(shownPalette + first + (count << 3) + j,
				currentPalette + first + (((count + y) % 8) << 3) +
				((j + x) % 8), sizeof(SDL_Color));

		}

	}

	if (direct) video.changePalette(shownPalette + first, first, amount);

	return;

}


/**
 * Create a new 1D parallaxing background palette effect.
 *
 * @param newFirst The first palette index to be affected
 * @param newAmount The number of palette indices to be affected
 * @param newSpeed The relative speed of the background
 * @param nextPE The next palette effect
 */
P1DPaletteEffect::P1DPaletteEffect
	(unsigned char newFirst, int newAmount, fixed newSpeed, PaletteEffect* nextPE) :
	PaletteEffect (nextPE) {

	first = newFirst;
	amount = newAmount;
	speed = newSpeed;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void P1DPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	SDL_Color* currentPalette;
	fixed position;
	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	currentPalette = video.getPalette();
	position = viewX + viewY;

	for (count = 0; count < amount; count++) {

		memcpy(shownPalette + first + count,
			currentPalette + first + ((count +
				(amount - 1 - (FTOI(MUL(position, speed)) % amount))) % amount),
			sizeof(SDL_Color));

	}

	if (direct) video.changePalette(shownPalette + first, first, amount);

	return;

}


/**
 * Create a new water palette effect.
 *
 * @param newDepth Water depth
 * @param nextPE The next palette effect
 */
WaterPaletteEffect::WaterPaletteEffect (fixed newDepth, PaletteEffect* nextPE)
	: PaletteEffect (nextPE) {

	depth = newDepth;

	return;

}


/**
 * Apply the palette effect.
 *
 * @param shownPalette The palette the effect will be applied to
 * @param direct Whether or not to apply the effect directly
 * @param mspf Ticks per frame
 * @param isStatic Whether the effect should advance after applying
 */
void WaterPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf, bool isStatic) {

	SDL_Color* currentPalette;
	int position, count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf, isStatic);

	currentPalette = video.getPalette();

	if (level) position = localPlayer->getLevelPlayer()->getY() - level->getWaterLevel();
	else if (jj2Level) position = localPlayer->getLevelPlayer()->getY() - jj2Level->getWaterLevel();
	else return;

	if (position <= 0) return;

	if (position < depth) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = FTOI(currentPalette[count].r *
				(1023 - DIV(position, depth)));
			shownPalette[count].g = FTOI(currentPalette[count].g *
				(1023 - DIV(position, depth)));
			shownPalette[count].b = FTOI(currentPalette[count].b *
				(1023 - DIV(position, depth)));

		}

	} else memset(shownPalette, 0, sizeof(SDL_Color) * 256);

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}
