
/**
 *
 * @file paletteeffects.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * 1st August 2009: Renamed palette.cpp to paletteeffects.cpp
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


#include "paletteeffects.h"
#include "video.h"

#include "jj2level/jj2level.h"
#include "level/level.h"
#include "player/jj2levelplayer.h"
#include "player/levelplayer.h"

#include <string.h>


PaletteEffect::PaletteEffect (PaletteEffect* nextPE) {

	next = nextPE;

	return;

}


PaletteEffect::~PaletteEffect () {

	if (next) delete next;

	return;

}


void PaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);

	return;

}


WhiteInPaletteEffect::WhiteInPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	whiteness = F1 + FH;

	return;

}


void WhiteInPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


	if (whiteness > F1) {

		memset(shownPalette, 255, sizeof(SDL_Color) * 256);

		whiteness -= ITOF(mspf) / duration;

	} else if (whiteness > 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = 255 -
				FTOI((255 - shownPalette[count].r) * (F1 - whiteness));
			shownPalette[count].g = 255 -
				FTOI((255 - shownPalette[count].g) * (F1 - whiteness));
			shownPalette[count].b = 255 -
				FTOI((255 - shownPalette[count].b) * (F1 - whiteness));

		}

		whiteness -= ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


FadeInPaletteEffect::FadeInPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	blackness = F1 + FH;

	return;

}


void FadeInPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


	if (blackness > F1) {

		memset(shownPalette, 0, sizeof(SDL_Color) * 256);

		blackness -= ITOF(mspf) / duration;

	} else if (blackness > 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r =
				FTOI(shownPalette[count].r * (F1 - blackness));
			shownPalette[count].g =
				FTOI(shownPalette[count].g * (F1 - blackness));
			shownPalette[count].b =
				FTOI(shownPalette[count].b * (F1 - blackness));

		}

		blackness -= ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


WhiteOutPaletteEffect::WhiteOutPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	whiteness = 0;

	return;

}


void WhiteOutPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


	if (whiteness > F1) {

		memset(shownPalette, 255, sizeof(SDL_Color) * 256);

	} else if (whiteness > 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = 255 -
				FTOI((255 - shownPalette[count].r) * (F1 - whiteness));
			shownPalette[count].g = 255 -
				FTOI((255 - shownPalette[count].g) * (F1 - whiteness));
			shownPalette[count].b = 255 -
				FTOI((255 - shownPalette[count].b) * (F1 - whiteness));

		}

		whiteness += ITOF(mspf) / duration;

	} else whiteness += ITOF(mspf) / duration;

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


FadeOutPaletteEffect::FadeOutPaletteEffect
	(int newDuration, PaletteEffect* nextPE) : PaletteEffect (nextPE) {

	duration = newDuration;
	blackness = -(F2 + F1);

	return;

}


void FadeOutPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);

	if (blackness > F1) {

		memset(shownPalette, 0, sizeof(SDL_Color) * 256);

	} else if (blackness > 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r =
				FTOI(shownPalette[count].r * (F1 - blackness));
			shownPalette[count].g =
				FTOI(shownPalette[count].g * (F1 - blackness));
			shownPalette[count].b =
				FTOI(shownPalette[count].b * (F1 - blackness));

		}

		blackness += ITOF(mspf) / duration;

	} else blackness += ITOF(mspf) / duration;

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


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


void FlashPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);

	if (progress < 0) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = FTOI((shownPalette[count].r * -progress) +
				(red * (progress + F1)));
			shownPalette[count].g = FTOI((shownPalette[count].g * -progress) +
				(green * (progress + F1)));
			shownPalette[count].b = FTOI((shownPalette[count].b * -progress) +
				(blue * (progress + F1)));

		}

		progress += ITOF(mspf) / duration;

	} else if (progress < F1) {

		for (count = 0; count < 256; count++) {

			shownPalette[count].r = FTOI((shownPalette[count].r * progress) +
				(red * (F1 - progress)));
			shownPalette[count].g = FTOI((shownPalette[count].g * progress) +
				(green * (F1 - progress)));
			shownPalette[count].b = FTOI((shownPalette[count].b * progress) +
				(blue * (F1 - progress)));

		}

		progress += ITOF(mspf) / duration;

	}

	if (direct) video.changePalette(shownPalette, 0, 256);

	return;

}


RotatePaletteEffect::RotatePaletteEffect
	(unsigned char newFirst, int newAmount, fixed newSpeed, PaletteEffect * nextPE) :
	PaletteEffect (nextPE) {

	first = newFirst;
	amount = newAmount;
	speed = newSpeed;
	position = 0;

	return;

}


void RotatePaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	SDL_Color* currentPalette;
	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


	currentPalette = video.getPalette();

	for (count = 0; count < amount; count++) {

		memcpy(shownPalette + first + count,
			currentPalette + first +
			((count + FTOI(position)) % amount), sizeof(SDL_Color));

	}

	position -= (mspf * speed) >> 10;
	while (position < 0) position += ITOF(amount);

	if (direct) video.changePalette(shownPalette + first, first, amount);

	return;

}


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


void SkyPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	int position, count, y;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


	position = viewY + (viewH << 9) - F4;

	if (canvasW > SW) y = ((canvasH - 1) / 100) + 1;
	else y = ((canvasH - 34) / 100) + 1;

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


P2DPaletteEffect::P2DPaletteEffect
	(unsigned char newFirst, int newAmount, fixed newSpeed, PaletteEffect* nextPE) :
	PaletteEffect (nextPE) {

	first = newFirst;
	amount = newAmount;
	speed = newSpeed;

	return;

}


void P2DPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	SDL_Color* currentPalette;
	int count, x, y, j;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


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


P1DPaletteEffect::P1DPaletteEffect
	(unsigned char newFirst, int newAmount, fixed newSpeed, PaletteEffect* nextPE) :
	PaletteEffect (nextPE) {

	first = newFirst;
	amount = newAmount;
	speed = newSpeed;

	return;

}


void P1DPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	SDL_Color* currentPalette;
	fixed position;
	int count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


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


WaterPaletteEffect::WaterPaletteEffect (fixed newDepth, PaletteEffect* nextPE)
	: PaletteEffect (nextPE) {

	depth = newDepth;

	return;

}


void WaterPaletteEffect::apply (SDL_Color* shownPalette, bool direct, int mspf) {

	SDL_Color* currentPalette;
	int position, count;

	// Apply the next palette effect
	if (next) next->apply(shownPalette, direct, mspf);


	currentPalette = video.getPalette();

	if (level) position = localPlayer->getLevelPlayer()->getY() - level->getWaterLevel();
	else if (jj2Level) position = localPlayer->getJJ2LevelPlayer()->getY() - jj2Level->getWaterLevel();
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


